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

// In TransientAnalysis.cpp

void TransientAnalysis::solve(Circuit &circuit)
{
    std::cout << "Starting transient analysis …\n"
              << "  stop time       : " << parameters_.stopTime_       << "\n"
              << "  output step     : " << parameters_.outputTimeStep_ << "\n"
              << "  start time      : " << parameters_.startTime_      << "\n"
              << "  max internal step: " << parameters_.maxInternalTimeStep_ << "\n";

    // (1) Get SUNDIALS context & number of eqns
    SUNContext sunctx = circuit.getSUNContext();
    if (!sunctx) {
        throw std::runtime_error("CRITICAL ERROR: SUNDIALS Context is NULL.");
    }
    long int num_equations = circuit.getNumEquations();
    if (num_equations <= 0) {
        throw std::runtime_error("CRITICAL ERROR: Invalid num_equations = "
                                 + std::to_string(num_equations));
    }

    // (2) Allocate y, yp, id
    N_Vector y_vec  = N_VNew_Serial(num_equations, sunctx);
    N_Vector yp_vec = N_VNew_Serial(num_equations, sunctx);
    N_Vector id_vec = N_VNew_Serial(num_equations, sunctx);
    if (!y_vec || !yp_vec || !id_vec) {
        std::cerr << "[ERROR] Failed to allocate N_Vectors.\n";
        if (y_vec ) N_VDestroy(y_vec);
        if (yp_vec) N_VDestroy(yp_vec);
        if (id_vec) N_VDestroy(id_vec);
        return;
    }
    std::cout << "[Info] Allocated N_Vectors of length " << num_equations << "\n";

    // (3) Fill initial conditions and ID vector
    circuit.getInitialConditions(y_vec, yp_vec);
    circuit.populateIdVector(id_vec);
    std::cout << "[Info] Initial conditions and ID vector populated.\n";

    // (4) Create IDA memory
    void *ida_mem = IDACreate(sunctx);
    if (!check_sundials_flag(ida_mem ? 0 : -1, "IDACreate")) {
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "[Info] IDA memory created.\n";

    // (5) Initialize IDA with residual callback
    int flag = IDAInit(ida_mem,
                       transientIDACallBack,
                       parameters_.startTime_,
                       y_vec,
                       yp_vec);
    if (!check_sundials_flag(flag, "IDAInit")) {
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "[Info] IDA initialized at t0 = " << parameters_.startTime_ << "\n";

    // (6) Attach circuit pointer
    flag = IDASetUserData(ida_mem, &circuit);
    if (!check_sundials_flag(flag, "IDASetUserData")) {
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }

    // (7) Set the ID vector (differential vs algebraic)
    flag = IDASetId(ida_mem, id_vec);
    if (!check_sundials_flag(flag, "IDASetId")) {
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "[Info] ID vector set.\n";

    // (8) Set solver tolerances
    sunrealtype reltol = 1.e-4, abstol = 1.e-6;
    flag = IDASStolerances(ida_mem, reltol, abstol);
    if (!check_sundials_flag(flag, "IDASStolerances")) {
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "[Info] Tolerances set (reltol=" << reltol
              << ", abstol=" << abstol << ").\n";

    // (9) Attach dense linear solver (or switch to sparse for large circuits)
    SUNMatrix A_mat = SUNDenseMatrix(num_equations, num_equations, sunctx);
    if (!A_mat) {
        std::cerr << "[ERROR] SUNDenseMatrix() failed.\n";
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    SUNLinearSolver LS_solver = SUNLinSol_Dense(y_vec, A_mat, sunctx);
    if (!LS_solver) {
        std::cerr << "[ERROR] SUNLinSol_Dense() failed.\n";
        SUNMatDestroy(A_mat);
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    flag = IDASetLinearSolver(ida_mem, LS_solver, A_mat);
    if (!check_sundials_flag(flag, "IDASetLinearSolver")) {
        SUNMatDestroy(A_mat);
        SUNLinSolFree(LS_solver);
        IDAFree(&ida_mem);
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "[Info] Dense linear solver attached.\n";

    // (10) Determine “max internal step” and “output step”
    sunrealtype maxstep = parameters_.maxInternalTimeStep_;
    if (maxstep <= 0.0) {
        maxstep = (parameters_.stopTime_ - parameters_.startTime_) / 1000.0;
        if (maxstep <= 0.0) maxstep = 1e-6;
    }
    IDASetMaxStep(ida_mem, maxstep);  // we ignore the return code for brevity

    sunrealtype outStep = parameters_.outputTimeStep_;
    if (outStep <= 0.0) {
        outStep = parameters_.stopTime_ - parameters_.startTime_;
        // i.e. just print final time if outputStep was zero
    }

    // (11) Compute a tiny “IC aim” time for the DAE solver to use
    sunrealtype t_i0 = parameters_.startTime_;
    sunrealtype t_ic = t_i0 + maxstep/100.0;
    if (t_ic <= t_i0) t_ic = t_i0 + 1e-9;

    std::cout << "[Info] Calling IDACalcIC at t = " << t_ic << " …\n";

    // (12) Use IDA_YA_YDP_INIT to solve both y(0) and y′(0) if needed
    int ic_flag = IDACalcIC(ida_mem, IDA_YA_YDP_INIT, t_ic);
    if (ic_flag != IDA_SUCCESS ) {
        std::cerr << "[WARN] IDACalcIC failed (flag=" << ic_flag << "). "
                  << "Proceeding with user‐supplied initial guess.\n";
    } else {
        std::cout << "[Info] IDACalcIC succeeded.\n";
    }

    // (13) Print header
    sunrealtype t_current = parameters_.startTime_;
    sunrealtype t_out     = t_current;

    std::cout << "\nTime";
    auto names = circuit.getOrderedUnknownNames();
    if ((int)names.size() != num_equations) {
        std::cerr << "[WARN] Name count (" << names.size()
                  << ") != equation count (" << num_equations << ")\n";
    }
    for (auto &nm : names) {
        std::cout << "," << nm;
    }
    std::cout << "\n";

    // (14) Print initial condition at t0
    circuit.printTransientResults(t_current, y_vec);

    // (15) Time‐march in “output” increments of outStep
    while (t_out < parameters_.stopTime_) {
        t_out += outStep;
        if (t_out > parameters_.stopTime_) {
            t_out = parameters_.stopTime_;
        }

        int solve_flag = IDASolve(ida_mem, t_out, &t_current, y_vec, yp_vec, IDA_NORMAL);
        if (solve_flag < 0 && solve_flag != IDA_WARNING && solve_flag != IDA_TSTOP_RETURN) {
            std::cerr << "[ERROR] IDASolve returned fatal code "
                      << solve_flag << ": " << IDAGetReturnFlagName(solve_flag) << "\n";
            break;
        }
        if (solve_flag == IDA_WARNING) {
            std::cerr << "[WARN] IDASolve warning: "
                      << IDAGetReturnFlagName(solve_flag) << "\n";
        }

        circuit.printTransientResults(t_current, y_vec);

        if (t_out >= parameters_.stopTime_) break;
    }

    std::cout << "\nTransient Analysis Finished.\n";

    // (16) Clean up in reverse order
    IDAFree(&ida_mem);
    SUNLinSolFree(LS_solver);
    SUNMatDestroy(A_mat);
    N_VDestroy(y_vec);
    N_VDestroy(yp_vec);
    N_VDestroy(id_vec);
}
