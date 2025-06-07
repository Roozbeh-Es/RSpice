#include "DCAnalysis.h"
#include "Circuit.h"
#include "Element.h"
#include <iostream>
#include <cmath>
#include <sundials/sundials_math.h> // For N_VWL2Norm
#include <sunmatrix/sunmatrix_dense.h> // For SM_ELEMENT_D

void DCAnalysis::solve(Circuit &circuit) {
    std::cout << "Starting DC Operating Point Analysis..." << std::endl;

    const int max_iters = 50;
    const double tol = 1e-9;
    const long int neq = circuit.getNumEquations();

    SUNContext ctx = circuit.getSUNContext();
    N_Vector y = N_VNew_Serial(neq, ctx);
    N_Vector F = N_VNew_Serial(neq, ctx);
    SUNMatrix J = SUNDenseMatrix(neq, neq, ctx);

    if (!y || !F || !J) {
        std::cerr << "[DC] ERROR: Failed to allocate vectors/matrix." << std::endl;
        // Clean up any successfully allocated memory before returning
        N_VDestroy(y);
        N_VDestroy(F);
        SUNMatDestroy(J);
        return;
    }

    // Set initial guess for y. Using the smart IC function is a good start.
    circuit.getInitialConditions(y, nullptr);

    std::cout << "Initial guess vector y:" << std::endl;
    for (int i = 0; i < neq; ++i) {
        std::cout << "  y[" << i << "] = " << NV_Ith_S(y, i) << std::endl;
    }

    for (int iter = 0; iter < max_iters; ++iter) {
        // Calculate residual F(y)
        N_VConst(0.0, F);
        for (const auto &el: circuit.getElements()) {
            el->DCStamp(y, F);
        }

        // Check for convergence
        double norm = N_VWL2Norm(F, F); // Using weighted L2 norm
        std::cout << "[DC] Iter " << iter << " residual norm = " << norm << std::endl;
        if (norm < tol) {
            std::cout << "Newton solver converged in " << iter + 1 << " iterations." << std::endl;
            break;
        }

        // Build Jacobian J = dF/dy via finite difference
        const double delta = 1e-8;
        for (int j = 0; j < neq; ++j) {
            sunrealtype orig = NV_Ith_S(y, j);
            NV_Ith_S(y, j) += delta; // Perturb y_j

            // Calculate F(y + delta)
            N_Vector F_perturb = N_VNew_Serial(neq, ctx);
            N_VConst(0.0, F_perturb);
            for (const auto &el: circuit.getElements()) {
                el->DCStamp(y, F_perturb);
            }

            // Calculate j-th column of Jacobian: J_j = (F(y+delta) - F(y)) / delta
            for (int i = 0; i < neq; ++i) {
                SM_ELEMENT_D(J, i, j) = (NV_Ith_S(F_perturb, i) - NV_Ith_S(F, i)) / delta;
            }

            NV_Ith_S(y, j) = orig; // Restore original y
            N_VDestroy(F_perturb);
        }

        // Solve the linear system: J * dx = -F
        SUNLinearSolver LS = SUNLinSol_Dense(y, J, ctx);
        N_Vector dx = N_VNew_Serial(neq, ctx);
        N_VScale(-1.0, F, dx); // dx = -F

        SUNLinSolSetup(LS, J);
        SUNLinSolSolve(LS, J, dx, dx, tol); // Solve for the update step dx

        // Update the solution: y = y + dx
        N_VLinearSum(1.0, y, 1.0, dx, y);

        // Cleanup for this iteration
        N_VDestroy(dx);
        SUNLinSolFree(LS);

        if (iter == max_iters - 1) {
            std::cerr << "DC analysis failed to converge after " << max_iters << " iterations." << std::endl;
        }
    }

    std::cout << "\n--- DC Operating Point Found ---" << std::endl;
    circuit.printDCResults(y);

    // Final cleanup
    N_VDestroy(y);
    N_VDestroy(F);
    SUNMatDestroy(J);
}
