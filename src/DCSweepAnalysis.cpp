#include "DCSweepAnalysis.h"
#include "Circuit.h"
#include "DCVoltageSource.h"
#include "DCCurrentSource.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sundials/sundials_math.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>

// Constructor implementation
DCSweepAnalysis::DCSweepAnalysis(const DCSweepParameters& parameters)
    : parameters_(parameters) {}

// The main solve method for DC Sweep
void DCSweepAnalysis::solve(Circuit &circuit) {
    std::cout << "Starting DC Sweep Analysis for source: " << parameters_.sourceName_ << std::endl;
    std::cout << "  Sweep from " << parameters_.startValue_ << " to " << parameters_.stopValue_
              << " in steps of " << parameters_.increment_ << std::endl;

    // --- 1. Find the element to be swept in the circuit ---
    Element* sweepElement = nullptr;
    for (const auto &el : circuit.getElements()) {
        if (el && el->getName() == parameters_.sourceName_) {
            sweepElement = el.get();
            break;
        }
    }
    if (!sweepElement) {
        throw std::runtime_error("DC Sweep source '" + parameters_.sourceName_ + "' not found in circuit.");
    }

    auto* sweepVoltageSource = dynamic_cast<DCVoltageSource*>(sweepElement);
    auto* sweepCurrentSource = dynamic_cast<DCCurrentSource*>(sweepElement);
    if (!sweepVoltageSource && !sweepCurrentSource) {
        throw std::runtime_error("Element '" + parameters_.sourceName_ + "' is not a sweepable DC source.");
    }

    // --- 2. Setup vectors and matrices for the Newton solver ---
    const long int neq = circuit.getNumEquations();
    SUNContext ctx = circuit.getSUNContext();
    N_Vector y = N_VNew_Serial(neq, ctx);
    N_Vector F = N_VNew_Serial(neq, ctx);
    SUNMatrix J = SUNDenseMatrix(neq, neq, ctx);

    if (!y || !F || !J) {
        throw std::runtime_error("[DC Sweep] Failed to allocate SUNDIALS vectors/matrix.");
    }

    // Use smart ICs for the very first starting guess
    circuit.getInitialConditions(y, nullptr);

    // --- 3. Print the output header ---
    std::cout << "\n" << std::left << std::setw(15) << sweepElement->getName();
    std::vector<std::string> unknownNames = circuit.getOrderedUnknownNames();
    for (const auto& name : unknownNames) {
        std::cout << "," << name;
    }
    std::cout << std::endl;

    // --- 4. The main sweep loop ---
    for (double sweepValue = parameters_.startValue_; sweepValue <= parameters_.stopValue_; sweepValue += parameters_.increment_) {

        // a. Update the value of the source being swept
        if (sweepVoltageSource) {
            sweepVoltageSource->setVoltage(sweepValue);
        } else {
            sweepCurrentSource->setCurrent(sweepValue);
        }

        // b. --- Your Newton-Raphson Solver Loop ---
        const int max_iters = 50;
        const double tol = 1e-9;
        bool converged = false;

        for (int iter = 0; iter < max_iters; ++iter) {
            // Calculate residual vector F(y)
            N_VConst(0.0, F);
            for (const auto &el : circuit.getElements()) {
                el->DCStamp(y, F);
            }

            // Check for convergence
            double norm = N_VWL2Norm(F, F);
            if (norm < tol) {
                converged = true;
                break;
            }

            // Build Jacobian J = dF/dy using finite differences
            const double delta = 1e-8;
            for (int j = 0; j < neq; ++j) {
                sunrealtype orig_yj = NV_Ith_S(y, j);
                NV_Ith_S(y, j) += delta;

                N_Vector F_perturb = N_VNew_Serial(neq, ctx);
                N_VConst(0.0, F_perturb);
                for (const auto &el : circuit.getElements()) {
                    el->DCStamp(y, F_perturb);
                }

                for (int i = 0; i < neq; ++i) {
                    sunrealtype* Jdata = SUNDenseMatrix_Data(J);
                    Jdata[i + j * neq] = (NV_Ith_S(F_perturb, i) - NV_Ith_S(F, i)) / delta;                }

                NV_Ith_S(y, j) = orig_yj;
                N_VDestroy(F_perturb);
            }

            // Solve linear system: J * dx = -F
            SUNLinearSolver LS = SUNLinSol_Dense(y, J, ctx);
            N_Vector dx = N_VNew_Serial(neq, ctx);
            N_VScale(-1.0, F, dx);

            SUNLinSolSetup(LS, J);
            SUNLinSolSolve(LS, J, dx, dx, tol);

            // Update solution: y_new = y_old + dx
            N_VLinearSum(1.0, y, 1.0, dx, y);

            N_VDestroy(dx);
            SUNLinSolFree(LS);
        }
        // --- End of Newton-Raphson Solver Loop ---

        // c. Print results for this step if it converged
        if (converged) {
            std::cout << std::left << std::setw(15) << std::fixed << std::setprecision(6) << sweepValue;
            sunrealtype* y_data = N_VGetArrayPointer(y);
            for (long int i = 0; i < neq; ++i) {
                std::cout << "," << std::scientific << y_data[i];
            }
            std::cout << std::endl;
        } else {
            std::cerr << "DC Sweep failed to converge at step: " << sweepElement->getName() << " = " << sweepValue << std::endl;
            break; // Stop the sweep if one point fails
        }
    }

    // --- 5. Final cleanup ---
    N_VDestroy(y);
    N_VDestroy(F);
    SUNMatDestroy(J);
}