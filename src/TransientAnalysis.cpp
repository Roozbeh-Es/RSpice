#include "TransientAnalysis.h"
#include "Circuit.h"
#include "Element.h"
#include <sundials/sundials_nvector.h>
#include "utility.h"
#include <vector>


int transientIDACallBack(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_residual, void *user_data) {
    auto *circuit = static_cast<Circuit *>(user_data);
    if (!circuit) {
        std::cerr << "ERROR: User_data in IDA residual callback is NULL!" << std::endl;
        return -1;
    }

    N_VConst(0.0, F_residual);

    for (const auto &element_ptr: circuit->getElements()) {
        if (element_ptr) {
            element_ptr->ResidualStamp(t, y, yp, F_residual);
        }
    }
    return 0;
}


void TransientAnalysis::solve(Circuit &circuit) {
    std::cout << "Starting transient analysis...\n"
              << "  stop time: " << parameters_.stopTime_ << "\n"
              << "  output step: " << parameters_.outputTimeStep_ << "\n"
              << "  start time: " << parameters_.startTime_ << "\n"
              << "  max internal step: " << parameters_.maxInternalTimeStep_ << "\n";

    // --- 1. SUNDIALS and MNA Setup ---
    SUNContext sunctx = circuit.getSUNContext();
    if (!sunctx) {
        throw std::runtime_error("CRITICAL ERROR: SUNDIALS Context is NULL.");
    }
    long int num_equations = circuit.getNumEquations();
    if (num_equations <= 0) {
        throw std::runtime_error("CRITICAL ERROR: Invalid number of equations.");
    }

    // --- 2. Allocate SUNDIALS objects ---
    N_Vector y_vec  = N_VNew_Serial(num_equations, sunctx);
    N_Vector yp_vec = N_VNew_Serial(num_equations, sunctx);
    N_Vector id_vec = N_VNew_Serial(num_equations, sunctx);
    void *ida_mem = nullptr;
    SUNMatrix A_mat = nullptr;
    SUNLinearSolver LS_solver = nullptr;

    if (!y_vec || !yp_vec || !id_vec) { // Allocation check
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        throw std::runtime_error("Failed to allocate N_Vectors.");
    }
    std::cout << "[Info] Allocated N_Vectors of length " << num_equations << "\n";

    // --- 3. Initialize IDA Solver ---
    circuit.getInitialConditions(y_vec, yp_vec);
    circuit.populateIdVector(id_vec);
    std::cout << "[Info] Initial conditions and ID vector populated.\n";

    ida_mem = IDACreate(sunctx);
    check_sundials_flag(ida_mem ? 0 : -1, "IDACreate");
    check_sundials_flag(IDAInit(ida_mem, transientIDACallBack, parameters_.startTime_, y_vec, yp_vec), "IDAInit");
    check_sundials_flag(IDASetUserData(ida_mem, &circuit), "IDASetUserData");
    check_sundials_flag(IDASetId(ida_mem, id_vec), "IDASetId");

    sunrealtype reltol = 1.e-4, abstol = 1.e-6;
    check_sundials_flag(IDASStolerances(ida_mem, reltol, abstol), "IDASStolerances");

    A_mat = SUNDenseMatrix(num_equations, num_equations, sunctx);
    LS_solver = SUNLinSol_Dense(y_vec, A_mat, sunctx);
    check_sundials_flag(A_mat && LS_solver ? 0 : -1, "SUNMatrix/SUNLinearSolver creation");
    check_sundials_flag(IDASetLinearSolver(ida_mem, LS_solver, A_mat), "IDASetLinearSolver");

    if (parameters_.maxInternalTimeStep_ > 0.0) {
        check_sundials_flag(IDASetMaxStep(ida_mem, parameters_.maxInternalTimeStep_), "IDASetMaxStep");
    }
    std::cout << "[Info] IDA solver fully initialized." << std::endl;

    // --- 4. Calculate Consistent Initial Conditions ---
    sunrealtype t_ic_aim = parameters_.startTime_ + 1e-6;
    if(parameters_.stopTime_ > parameters_.startTime_) {
       t_ic_aim = parameters_.startTime_ + std::min((sunrealtype)(parameters_.stopTime_ - parameters_.startTime_)/1000.0, 1e-6);
    }
    std::cout << "[Info] Calling IDACalcIC, aiming for t = " << t_ic_aim << "...\n";
    check_sundials_flag(IDACalcIC(ida_mem, IDA_YA_YDP_INIT, t_ic_aim), "IDACalcIC");
    std::cout << "[Info] IDACalcIC succeeded.\n";

    // --- 5. Corrected Simulation Loop ---
    sunrealtype t_current = parameters_.startTime_;
    sunrealtype t_out_next = parameters_.startTime_;
    sunrealtype output_step = parameters_.outputTimeStep_;

    // Handle case where output step is zero or invalid
    if (output_step <= 0) {
        output_step = parameters_.stopTime_;
        std::cout << "[Info] outputTimeStep is 0. Only initial and final points will be printed.\n";
    }

    // Print CSV Header
    std::cout << "\nTime";
    std::vector<std::string> unknown_names = circuit.getOrderedUnknownNames();
    for (const auto &name: unknown_names) {
        std::cout << "," << name;
    }
    std::cout << std::endl;

    circuit.printTransientResults(t_current, y_vec);

    while (t_current < parameters_.stopTime_) {

        int flag = IDASolve(ida_mem, parameters_.stopTime_, &t_current, y_vec, yp_vec, IDA_ONE_STEP);
        if (flag < 0) { // On any fatal error, stop.
            check_sundials_flag(flag, "IDASolve");
            break;
        }


        while (t_out_next + output_step <= t_current) {
            t_out_next += output_step;


            N_Vector y_interpolated = N_VNew_Serial(num_equations, sunctx);
            if (check_sundials_flag(IDAGetDky(ida_mem, t_out_next, 0, y_interpolated), "IDAGetDky")) {
                circuit.printTransientResults(t_out_next, y_interpolated);
            }
            N_VDestroy(y_interpolated);
        }
    }

    if (t_current < parameters_.stopTime_) {
         circuit.printTransientResults(parameters_.stopTime_, y_vec);
    }

    std::cout << "\nTransient Analysis Finished.\n";

    // --- 6. Cleanup ---
    IDAFree(&ida_mem);
    SUNLinSolFree(LS_solver);
    SUNMatDestroy(A_mat);
    N_VDestroy(y_vec);
    N_VDestroy(yp_vec);
    N_VDestroy(id_vec);
}
