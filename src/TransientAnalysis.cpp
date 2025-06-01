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
            << ", output stem: " << parameters_.outputTimeStep_
            << ", start time : " << parameters_.startTime_
            << ", max internal step: " << parameters_.maxInternalTimeStep_ << std::endl;


    SUNContext suncntx = circuit.getSUNContext();

    if (!suncntx) {
        throw std::runtime_error("CRITICAL ERROR: SUNDIALS Context is NULL from Circuit object.");
    }

    long int num_equations = circuit.getNumEquations();

    N_Vector y_vec = N_VNew_Serial(num_equations, suncntx);
    N_Vector yp_vec = N_VNew_Serial(num_equations, suncntx);


    if (!y_vec || !yp_vec) {
        std::cerr << "SUNDIALS Error: Failed to allocate N_Vectors y or yp." << std::endl;
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }

    std::cout << "N_Vectors y and yp allocated with size: " << num_equations << std::endl;


    circuit.getInitialConditions(y_vec, yp_vec);
    std::cout << "initial conditions set" << std::endl;
    void *ida_mem = IDACreate(suncntx);
    if (!ida_mem) {
        std::cerr << "SUNDIALS Error: Failed to create IDA!" << std::endl;
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }

    std::cout << "IDA memory created" << std::endl;

    if (!check_sundials_flag(IDAInit(ida_mem, transientIDACallBack, parameters_.startTime_, y_vec, yp_vec),
                             "IDAInit")) {
        IDAFree(&ida_mem);
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }
    std::cout << "IDA solver initialized with callback at t0 - " << parameters_.startTime_ << std::endl;

    sunrealtype reltol = 1.0e-4;
    sunrealtype abstol_scalar = 1.0e-6;

    if (!check_sundials_flag(IDASStolerances(ida_mem, reltol, abstol_scalar), "IDASStolerances")) {
        IDAFree(&ida_mem);
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }
    std::cout << "IDA tolerances set (reltol=" << reltol << ", abstol=" << abstol_scalar << ")." << std::endl;


    SUNMatrix A_mat = SUNDenseMatrix(num_equations, num_equations, suncntx);
    SUNLinearSolver LS_solver = SUNLinSol_Dense(y_vec, A_mat, suncntx);

    if (!A_mat || !LS_solver) {
        std::cerr << "SUNDIALS Error: Failed to create SUNMatrix or SUNLinearSolver." << std::endl;
        SUNMatDestroy(A_mat);
        SUNLinSolFree(LS_solver);
        IDAFree(&ida_mem);
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }
    if (!check_sundials_flag(IDASetLinearSolver(ida_mem, LS_solver, A_mat), "IDASetLinearSolver")) {
        SUNMatDestroy(A_mat);
        SUNLinSolFree(LS_solver);
        IDAFree(&ida_mem);
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }
    std::cout << "IDA linear solver (Dense) set." << std::endl;

    if (parameters_.maxInternalTimeStep_ > 0.0) {
        if (!check_sundials_flag(IDASetMaxStep(ida_mem, parameters_.maxInternalTimeStep_), "IDASetMaxStep")) {
            std::cerr << "Warning: Failed to set IDA max step size." << std::endl;
        } else {
            std::cout << "IDA max internal step size set to: " << parameters_.maxInternalTimeStep_ << std::endl;
        }
    }

    sunrealtype t_first_calc_ic_aim = parameters_.startTime_ + 1e-9;
    if (parameters_.outputTimeStep_ > 0.0) {
        t_first_calc_ic_aim = parameters_.startTime_ + std::min(parameters_.outputTimeStep_,
                                                                (parameters_.maxInternalTimeStep_ > 0
                                                                     ? parameters_.maxInternalTimeStep_
                                                                     : parameters_.outputTimeStep_)) / 100.0;
    }

    std::cout << "Calculating consistent initial conditions, " << std::endl;


    sunrealtype t_current = parameters_.startTime_;
    sunrealtype t_out_next_print = parameters_.startTime_;

    std::cout << "\nTime";
    std::vector<std::string> unknown_names = circuit.getOrderedUnknownNames();
    for(const auto& name : unknown_names) {
        std::cout << "," << name;
    }
    std::cout << std::endl;

    if (parameters_.outputTimeStep_ == 0 && parameters_.startTime_ == parameters_.stopTime_) {
        circuit.printTransientResults(t_current, y_vec);
    } else if (parameters_.outputTimeStep_ > 0) {
         circuit.printTransientResults(t_current, y_vec);
         t_out_next_print += parameters_.outputTimeStep_;
    } else if (parameters_.outputTimeStep_ == 0 && parameters_.startTime_ < parameters_.stopTime_) {
        circuit.printTransientResults(t_current, y_vec);
        std::cerr << "Warning: outputTimeStep is zero, only initial conditions will be printed for a range." << std::endl;
        t_out_next_print = parameters_.stopTime_ + 1.0;
    }


    while (t_current < parameters_.stopTime_) {
        sunrealtype t_solve_target_for_ida = t_out_next_print;
        if (t_solve_target_for_ida > parameters_.stopTime_) {
            t_solve_target_for_ida = parameters_.stopTime_;
        }
         if (t_solve_target_for_ida <= t_current) {
            if (t_current < parameters_.stopTime_) {
                 t_solve_target_for_ida = parameters_.stopTime_;
            } else {
                 break;
            }
        }

        int flag = IDASolve(ida_mem, t_solve_target_for_ida, &t_current, y_vec, yp_vec, IDA_NORMAL);

        if (!check_sundials_flag(flag, "IDASolve")) {
            break; // Error in solver
        }


        circuit.printTransientResults(t_current, y_vec);

        if (parameters_.outputTimeStep_ <= 0) break;
        t_out_next_print += parameters_.outputTimeStep_;

        if (t_current >= parameters_.stopTime_) break;
    }

    std::cout << "\nTransient Analysis Finished." << std::endl;

    IDAFree(&ida_mem);
    SUNLinSolFree(LS_solver);
    SUNMatDestroy(A_mat);
    N_VDestroy(y_vec);
    N_VDestroy(yp_vec);
}
