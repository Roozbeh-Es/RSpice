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
            << ", output stop: " << parameters_.outputTimeStep_ // Typo: should be "step"
            << ", start time : " << parameters_.startTime_
            << ", max internal step: " << parameters_.maxInternalTimeStep_ << std::endl;

    SUNContext sunctx = circuit.getSUNContext();

    if (!sunctx) { // Ensure consistent variable name (suncntx vs sunctx)
        throw std::runtime_error("CRITICAL ERROR: SUNDIALS Context is NULL from Circuit object.");
    }

    long int num_equations = circuit.getNumEquations();
    if (num_equations <= 0) { // Added check for num_equations
        throw std::runtime_error("CRITICAL ERROR: Number of equations from Circuit is invalid: " + std::to_string(num_equations));
    }


    N_Vector y_vec = N_VNew_Serial(num_equations, sunctx); // Corrected: sunctx
    N_Vector yp_vec = N_VNew_Serial(num_equations, sunctx); // Corrected: sunctx

    if (!y_vec || !yp_vec) {
        std::cerr << "SUNDIALS Error: Failed to allocate N_Vectors y or yp." << std::endl;
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }
    std::cout << "N_Vectors y and yp allocated with size: " << num_equations << std::endl;

    circuit.getInitialConditions(y_vec, yp_vec);
    std::cout << "Initial conditions set by circuit.getInitialConditions." << std::endl;
    // You might want to print the initial y_vec and yp_vec here for debugging

    void *ida_mem = IDACreate(sunctx); // Corrected: sunctx
    if (!ida_mem) { // IDACreate returns NULL on failure
        std::cerr << "SUNDIALS Error: IDACreate failed!" << std::endl;
        N_VDestroy(y_vec);
        N_VDestroy(yp_vec);
        return;
    }
    std::cout << "IDA memory created." << std::endl;

    if (!check_sundials_flag(IDAInit(ida_mem, transientIDACallBack, parameters_.startTime_, y_vec, yp_vec), "IDAInit")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); return;
    }
    std::cout << "IDA solver initialized with callback at t0 = " << parameters_.startTime_ << std::endl;

    // --- ADDED: SET USER DATA ---
    if (!check_sundials_flag(IDASetUserData(ida_mem, &circuit), "IDASetUserData")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); return;
    }
    std::cout << "IDA user data set to circuit object." << std::endl;

    sunrealtype reltol = 1.0e-4;
    sunrealtype abstol_scalar = 1.0e-6;
    if (!check_sundials_flag(IDASStolerances(ida_mem, reltol, abstol_scalar), "IDASStolerances")) {
        IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); return;
    }
    std::cout << "IDA tolerances set (reltol=" << reltol << ", abstol=" << abstol_scalar << ")." << std::endl;

    SUNMatrix A_mat = SUNDenseMatrix(num_equations, num_equations, sunctx); // Corrected: sunctx
    SUNLinearSolver LS_solver = SUNLinSol_Dense(y_vec, A_mat, sunctx); // Corrected: sunctx

    if (!A_mat || !LS_solver) {
        std::cerr << "SUNDIALS Error: Failed to create SUNMatrix or SUNLinearSolver." << std::endl;
        SUNMatDestroy(A_mat); SUNLinSolFree(LS_solver); IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); return;
    }
    if (!check_sundials_flag(IDASetLinearSolver(ida_mem, LS_solver, A_mat), "IDASetLinearSolver")) {
        SUNMatDestroy(A_mat); SUNLinSolFree(LS_solver); IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec); return;
    }
    std::cout << "IDA linear solver (Dense) set." << std::endl;

    if (parameters_.maxInternalTimeStep_ > 0.0) {
        if (!check_sundials_flag(IDASetMaxStep(ida_mem, parameters_.maxInternalTimeStep_), "IDASetMaxStep")) {
            std::cerr << "Warning: Failed to set IDA max step size." << std::endl;
        } else {
            std::cout << "IDA max internal step size set to: " << parameters_.maxInternalTimeStep_ << std::endl;
        }
    }

    // --- ADDED: CALCULATE CONSISTENT INITIAL CONDITIONS ---
    sunrealtype t_first_calc_ic_aim = parameters_.startTime_;
     if (parameters_.stopTime_ > parameters_.startTime_) { // Only aim ahead if there's a duration
        sunrealtype first_step_hint = parameters_.outputTimeStep_ > 0 ? parameters_.outputTimeStep_ : (parameters_.stopTime_ - parameters_.startTime_);
        if (parameters_.maxInternalTimeStep_ > 0) {
            first_step_hint = std::min(first_step_hint, static_cast<sunrealtype>(parameters_.maxInternalTimeStep_));
        }
        // Ensure the aim is slightly ahead but not too far, and valid
        t_first_calc_ic_aim = parameters_.startTime_ + std::min(first_step_hint / 100.0, static_cast<sunrealtype>((parameters_.stopTime_ - parameters_.startTime_) / 2.0));
         if (t_first_calc_ic_aim <= parameters_.startTime_) {
            t_first_calc_ic_aim = parameters_.startTime_ + 1e-9; // A very small nudge if stopTime > startTime
         }
    }
    // If startTime == stopTime (single point analysis), t_first_calc_ic_aim correctly remains parameters_.startTime_


    std::cout << "Calculating consistent initial conditions (IDACalcIC), IDA aiming for t_ic_aim = " << t_first_calc_ic_aim << std::endl;
    if (!check_sundials_flag(IDACalcIC(ida_mem, IDA_YA_YDP_INIT, t_first_calc_ic_aim), "IDACalcIC")) {
        std::cerr << "CRITICAL ERROR: IDACalcIC failed. Check DAE model, initial guesses, and DAE index." << std::endl;
        SUNMatDestroy(A_mat); SUNLinSolFree(LS_solver); IDAFree(&ida_mem); N_VDestroy(y_vec); N_VDestroy(yp_vec);
        return;
    }
    std::cout << "Consistent initial conditions calculated successfully." << std::endl;
    // At this point, y_vec and yp_vec have been updated by IDACalcIC to be consistent.
    // You might want to print them here for debugging.
    std::cout << "Consistent y(t0) after IDACalcIC:" << std::endl;
    circuit.printTransientResults(parameters_.startTime_, y_vec);


    // --- SIMULATION LOOP ---
    sunrealtype t_current = parameters_.startTime_;
    sunrealtype t_out_next_print = parameters_.startTime_;

    std::cout << "\nTime";
    std::vector<std::string> unknown_names = circuit.getOrderedUnknownNames();
    for(const auto& name : unknown_names) {
        std::cout << "," << name;
    }
    std::cout << std::endl;

    // Output initial consistent conditions if startTime is an output point
    // This logic ensures t0 results (which are now consistent) are printed if outputTimeStep allows
    bool single_point_analysis = (parameters_.outputTimeStep_ == 0 && parameters_.startTime_ == parameters_.stopTime_);
    if (single_point_analysis) {
        // Already printed above after IDACalcIC if startTime_ == stopTime_
        if (parameters_.startTime_ != t_current) { // If IDACalcIC changed t_current (should not for t0)
             circuit.printTransientResults(t_current, y_vec);
        }
    } else if (parameters_.outputTimeStep_ > 0) {
         // Already printed initial consistent conditions at t_current = parameters_.startTime_
         t_out_next_print = parameters_.startTime_ + parameters_.outputTimeStep_;
    } else if (parameters_.outputTimeStep_ == 0 && parameters_.startTime_ < parameters_.stopTime_) {
        // Already printed initial. Warning for no further output.
        std::cerr << "Warning: outputTimeStep is zero for a ranged simulation. Only consistent initial conditions will be printed." << std::endl;
        t_out_next_print = parameters_.stopTime_ + 1.0; // To exit loop after one iteration if not already at stopTime
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
            break;
        }

        circuit.printTransientResults(t_current, y_vec);

        if (parameters_.outputTimeStep_ <= 0 && parameters_.startTime_ == parameters_.stopTime_) break; // Single point was done
        if (parameters_.outputTimeStep_ <= 0 && parameters_.startTime_ < parameters_.stopTime_) break; // Only IC was printed

        t_out_next_print += parameters_.outputTimeStep_;
        if (t_out_next_print <= t_current && t_current < parameters_.stopTime_) { // Safety for very small outputTimeStep_
            t_out_next_print = t_current + parameters_.outputTimeStep_;
        }

        if (t_current >= parameters_.stopTime_) break;
    }

    std::cout << "\nTransient Analysis Finished." << std::endl;

    // --- L. Cleanup ---
    IDAFree(&ida_mem);
    SUNLinSolFree(LS_solver);
    SUNMatDestroy(A_mat);
    N_VDestroy(y_vec);
    N_VDestroy(yp_vec);
    // sunctx is managed by Circuit, not freed here.
}
