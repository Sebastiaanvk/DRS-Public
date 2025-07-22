#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"



int main(){
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
//    env.set("LogFile", "output/logs/gurobi.log");
    env.set("LogFile", "/tmp/gurobi.log");
    env.start();

    try{ 
        GRBModel model(env);  
        GRBVar x = model.addVar(-GRB_INFINITY,GRB_INFINITY, 1, 'C', "x");
        model.addConstr(x <= 10, "c0");
        model.optimize();
        std::cout << "License is working correctly." << std::endl;
    } catch(GRBException& e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    return 0;
}
