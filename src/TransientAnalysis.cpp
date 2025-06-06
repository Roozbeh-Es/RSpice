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
    std::cout << "Starting transient analysis ..." << std::endl;
    std::cout << "stop time : " << parameters_.stopTime_
            << ", output step: " << parameters_.outputTimeStep_
            << ", start time : " << parameters_.startTime_
            << ", max internal step: " << parameters_.maxInternalTimeStep_ << std::endl;

    SUNContext sunctx = circuit.getSUNContext();
    if (!sunctx) {
        throw std::runtime_error("CRITICAL ERROR: SUNDIALS Context is NULL from Circuit object.");
    }

    long int num_equations = circuit.getNumEquations();
    if (num_equations <= 0) {
        throw std::runtime_error("CRITICAL ERROR: Number of equations from Circuit is invalid: " + std::to_string(num_equations));
    }

    N_Vector y_vec = N_VNew_Serial(num_equations, sunctx);
    N_Vector yp_vec = N_VNew_Serial(num_equations, sunctx);
    N_Vector id_vec = N_VNew_Serial(num_equations, sunctx);
    void *ida_mem = nullptr;
    SUNMatrix A_mat = nullptr;
    SUNLinearSolver LS_solver = nullptr;

    if (!y_vec || !yp_vec || !id_vec) {
        std::cerr << "SUNDIALS Error: Failed to allocate N_Vectors." << std::endl;
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "N_Vectors y, yp, and id allocated with size: " << num_equations << std::endl;

    circuit.getInitialConditions(y_vec, yp_vec);
    circuit.populateIdVector(id_vec);
    std::cout << "Initial conditions and ID vector populated." << std::endl;

    ida_mem = IDACreate(sunctx);
    if (!check_sundials_flag(ida_mem ? 0 : -1, "IDACreate")) {
        N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    std::cout << "IDA memory created." << std::endl;

    if (!check_sundials_flag(IDAInit(ida_mem, transientIDACallBack, parameters_.startTime_, y_vec, yp_vec), "IDAInit")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    std::cout << "IDA solver initialized with callback at t0 = " << parameters_.startTime_ << std::endl;

    if (!check_sundials_flag(IDASetUserData(ida_mem, &circuit), "IDASetUserData")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    std::cout << "IDA user data set to circuit object." << std::endl;

    if (!check_sundials_flag(IDASetId(ida_mem, id_vec), "IDASetId")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    std::cout << "IDA ID vector set." << std::endl;

    sunrealtype reltol = 1.0e-4;
    sunrealtype abstol_scalar = 1.0e-6;
    if (!check_sundials_flag(IDASStolerances(ida_mem, reltol, abstol_scalar), "IDASStolerances")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    std::cout << "IDA tolerances set (reltol=" << reltol << ", abstol=" << abstol_scalar << ")." << std::endl;

    A_mat = SUNDenseMatrix(num_equations, num_equations, sunctx);
    LS_solver = SUNLinSol_Dense(y_vec, A_mat, sunctx);
    if (!check_sundials_flag(A_mat && LS_solver ? 0 : -1, "SUNMatrix/SUNLinearSolver creation")) {
        SUNMatDestroy(A_mat); SUNLinSolFree(LS_solver); IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    if (!check_sundials_flag(IDASetLinearSolver(ida_mem, LS_solver, A_mat), "IDASetLinearSolver")) {
        SUNMatDestroy(A_mat); SUNLinSolFree(LS_solver); IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec); return;
    }
    std::cout << "IDA linear solver (Dense) set." << std::endl;

    if (parameters_.maxInternalTimeStep_ > 0.0) {
        if (!check_sundials_flag(IDASetMaxStep(ida_mem, parameters_.maxInternalTimeStep_), "IDASetMaxStep")) {
            std::cerr << "Warning: Failed to set IDA max step size." << std::endl;
        } else {
            std::cout << "IDA max internal step size set to: " << parameters_.maxInternalTimeStep_ << std::endl;
        }
    }

    sunrealtype t_first_calc_ic_aim = parameters_.startTime_;
    if (parameters_.stopTime_ > parameters_.startTime_) {
        sunrealtype first_step_hint = parameters_.outputTimeStep_ > 0 ? parameters_.outputTimeStep_ : (parameters_.stopTime_ - parameters_.startTime_);
        if (parameters_.maxInternalTimeStep_ > 0) {
            first_step_hint = std::min(first_step_hint, static_cast<sunrealtype>(parameters_.maxInternalTimeStep_));
        }
        t_first_calc_ic_aim = parameters_.startTime_ + std::min(first_step_hint / 100.0, static_cast<sunrealtype>((parameters_.stopTime_ - parameters_.startTime_) / 2.0));
        if (t_first_calc_ic_aim <= parameters_.startTime_) {
            t_first_calc_ic_aim = parameters_.startTime_ + 1e-9;
        }
    }

    std::cout << "Calculating consistent initial conditions (IDACalcIC), IDA aiming for t_ic_aim = " << t_first_calc_ic_aim << std::endl;
    if (!check_sundials_flag(IDACalcIC(ida_mem, IDA_YA_YDP_INIT, t_first_calc_ic_aim), "IDACalcIC")) {
        std::cerr << "CRITICAL ERROR: IDACalcIC failed. Check DAE model, initial guesses, and DAE index." << std::endl;
        SUNMatDestroy(A_mat); SUNLinSolFree(LS_solver); IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); N_VDestroy(id_vec);
        return;
    }
    std::cout << "Consistent initial conditions calculated successfully." << std::endl;
    std::cout << "Consistent y(t0) after IDACalcIC:" << std::endl;
    circuit.printTransientResults(parameters_.startTime_, y_vec);

    sunrealtype t_current = parameters_.startTime_;
    sunrealtype t_out_next_print = parameters_.startTime_;

    std::cout << "\nTime";
    std::vector<std::string> unknown_names = circuit.getOrderedUnknownNames();
    for (const auto &name: unknown_names) {
        std::cout << "," << name;
    }
    std::cout << std::endl;

    circuit.printTransientResults(t_current, y_vec);

    if (parameters_.outputTimeStep_ > 0) {
        t_out_next_print += parameters_.outputTimeStep_;
    } else if (parameters_.startTime_ < parameters_.stopTime_) {
        std::cerr << "Warning: outputTimeStep is zero for a ranged simulation. Only initial conditions will be printed." << std::endl;
        t_out_next_print = parameters_.stopTime_ + 1.0;
    }

    while (t_current < parameters_.stopTime_) {
        sunrealtype t_solve_target = std::min(t_out_next_print, static_cast<sunrealtype>(parameters_.stopTime_));
        int flag = IDASolve(ida_mem, t_solve_target, &t_current, y_vec, yp_vec, IDA_NORMAL);

        if (!check_sundials_flag(flag, "IDASolve")) {
            break;
        }

        if (t_current >= t_out_next_print) {
            circuit.printTransientResults(t_out_next_print, y_vec);
            if (parameters_.outputTimeStep_ > 0) {
                 t_out_next_print += parameters_.outputTimeStep_;
            }
        }
    }

    if (t_current < parameters_.stopTime_) {
        circuit.printTransientResults(parameters_.stopTime_, y_vec);
    }

    std::cout << "\nTransient Analysis Finished." << std::endl;

    IDAFree(&ida_mem);
    SUNLinSolFree(LS_solver);
    SUNMatDestroy(A_mat);
    N_VDestroy(y_vec);
    N_VDestroy(yp_vec);
    N_VDestroy(id_vec);
}
