#include <iostream>
#include <scip/scip.h>
#include "model.hpp"
#include "model_data.hpp"
#include "scip/scipdefplugins.h"

int main()
{

    ModelData data;
    if (!data.loadFromFile("data/smaller_0.json"))
    {
        return 1;
    }

    // Build model
    ModelBuilder model = ModelBuilder(data);
    if (model.buildModel() != SCIP_OKAY)
    {
        std::cerr << "Model build failed!\n";
        model.~ModelBuilder();
        return 1;
    }

    // Solve
    model.solveModel();

    // // Clean up
    // for (auto var : builder.getAllVars()) {
    //     SCIPreleaseVar(scip, &var);
    // }

    // SCIPfree(&scip);
    return 0;
}
