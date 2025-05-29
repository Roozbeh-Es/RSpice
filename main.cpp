#include <iostream>
#include <vector>
#include <cmath>
#include <string>

// SUNDIALS Headers
#include <ida/ida.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_context.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct CircuitData {
    double R1_val = 1000.0;
    double C1_val = 1.0e-6;
    double V_amp = 5.0;
    double V_freq = 1000.0;
};

int residual_function(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F, void* user_data) {
    CircuitData* circuit = static_cast<CircuitData*>(user_data);
    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data = N_VGetArrayPointer(F);

    sunrealtype V1 = y_data[0];
    sunrealtype V2 = y_data[1];
    sunrealtype I_Vsine = y_data[2];
    sunrealtype V2_prime = yp_data[1];

    double V_source_t = circuit->V_amp * sin(2.0 * M_PI * circuit->V_freq * t);

    F_data[0] = (V1 - V2) / circuit->R1_val + I_Vsine;
    F_data[1] = (V2 - V1) / circuit->R1_val + circuit->C1_val * V2_prime;
    F_data[2] = V1 - V_source_t;

    return 0;
}

int main() {
    SUNContext sunctx;
    SUNContext_Create(NULL, &sunctx);

    CircuitData circuit;
    void* user_data = &circuit;

    long int num_equations = 3;
    N_Vector y = NULL;
    N_Vector yp = NULL;
    void* ida_mem = NULL;
    SUNMatrix A = NULL;
    SUNLinearSolver LS = NULL;

    y = N_VNew_Serial(num_equations, sunctx);
    yp = N_VNew_Serial(num_equations, sunctx);
    ida_mem = IDACreate(sunctx);

    N_VConst(0.0, y);
    N_VConst(0.0, yp);

    sunrealtype t0 = 0.0;
    IDAInit(ida_mem, residual_function, t0, y, yp);

    sunrealtype rel_tol = 1e-4;
    sunrealtype abs_tol = 1e-6;
    IDASStolerances(ida_mem, rel_tol, abs_tol);

    IDASetUserData(ida_mem, user_data);

    A = SUNDenseMatrix(num_equations, num_equations, sunctx);
    LS = SUNLinSol_Dense(y, A, sunctx);
    IDASetLinearSolver(ida_mem, LS, A);

    sunrealtype t_final = 0.005;
    sunrealtype dt = 0.0001;

    IDACalcIC(ida_mem, IDA_YA_YDP_INIT, t0 + dt);

    sunrealtype t_out = t0;
    std::cout << "Time,V1,V2,I_Vsine" << std::endl;

    while (t_out < t_final) {
        sunrealtype t_return;
        if (IDASolve(ida_mem, t_out + dt, &t_return, y, yp, IDA_NORMAL) < 0) break;

        sunrealtype* y_data = N_VGetArrayPointer(y);
        std::cout << t_return << "," << y_data[0] << "," << y_data[1] << "," << y_data[2] << std::endl;

        t_out = t_return;
    }

    IDAFree(&ida_mem);
    SUNLinSolFree(LS);
    SUNMatDestroy(A);
    N_VDestroy(y);
    N_VDestroy(yp);
    SUNContext_Free(&sunctx);

    return 0;
}
