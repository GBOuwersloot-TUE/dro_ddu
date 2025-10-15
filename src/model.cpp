#include <iostream>
#include <string>
#include <scip/scip.h>
#include "model.hpp"
#include "consts.hpp"
#include "scip/scipdefplugins.h"
#include "objscip/objscip.h"

ModelBuilder::ModelBuilder(const ModelData &data) : data_(data)
{
    constructModel();
}

SCIP_RETCODE ModelBuilder::constructModel()
{

    SCIP_CALL(SCIPcreate(&scip_));
    SCIP_CALL(SCIPincludeDefaultPlugins(scip_));
    SCIP_CALL(SCIPcreateProbBasic(scip_, "MasterProblemSCIP"));

    return SCIP_OKAY;
}

ModelBuilder::~ModelBuilder()
{
    deconstructModel();
}

SCIP_RETCODE ModelBuilder::deconstructModel()
{
    SCIP_CALL(SCIPfree(&scip_));
    BMScheckEmptyMemory();

    return SCIP_OKAY;
}

SCIP_RETCODE ModelBuilder::buildModel()
{
    SCIP_CALL(addVars());
    SCIP_CALL(addConstraints());
    SCIP_CALL(SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE));

    return SCIP_OKAY;
}

SCIP_RETCODE ModelBuilder::addVars()
{
    std::string varName;
    float varObj;
    for (int a = 0; a < data_.arcs.size(); ++a)
    {
        for (int t = 0; t < data_.timePeriods; ++t)
        {
            varName = "x[" + std::to_string(a) + "," + std::to_string(t) + "]";
            varObj = data_.arcs[a].buildCost[t];
            if (t < data_.timePeriods - 1)
            {
                varObj -= data_.arcs[a].buildCost[t + 1];
            }
            addVar(varName, 0.0, 1.0, varObj, SCIP_VARTYPE_BINARY);

            varName = "Phi1[" + std::to_string(a) + "," + std::to_string(t) + "]";
            varObj = data_.omega[t] / data_.arcs.size();
            addVar(varName, 0.0, BETA_UB, varObj, SCIP_VARTYPE_CONTINUOUS);

            varName = "Phi2[" + std::to_string(a) + "," + std::to_string(t) + "]";
            varObj = -data_.omega[t] / data_.arcs.size();
            addVar(varName, 0.0, BETA_UB, varObj, SCIP_VARTYPE_CONTINUOUS);
        }
    }
    for (int t = 0; t < data_.timePeriods; ++t)
    {
        for (int k = 0; k < data_.commodities; ++k)
        {

            varName = "beta1[" + std::to_string(t) + "," + std::to_string(k) + "]";
            varObj = data_.muBar[t][k] + data_.epsilon[t][k];
            addVar(varName, 0.0, BETA_UB, varObj, SCIP_VARTYPE_CONTINUOUS);

            varName = "beta2[" + std::to_string(t) + "," + std::to_string(k) + "]";
            varObj = -data_.muBar[t][k] + data_.epsilon[t][k];
            addVar(varName, 0.0, BETA_UB, varObj, SCIP_VARTYPE_CONTINUOUS);
        }
    }
    return SCIP_OKAY;
}

SCIP_RETCODE ModelBuilder::addVar(const std::string &varName, double lb, double ub, double obj, SCIP_VARTYPE varType)
{
    SCIP_VAR *var;

    SCIP_CALL(SCIPcreateVarBasic(scip_, &var, varName.c_str(),
                                 lb, ub, obj, varType));
    SCIP_CALL(SCIPaddVar(scip_, var));
    variables_.push_back(var);

    SCIP_CALL(SCIPreleaseVar(scip_, &var));

    return SCIP_OKAY;
}

SCIP_RETCODE ModelBuilder::addConstraints()
{
    std::string phiVarName;
    std::string xVarName;
    std::string betaVarName;
    std::string consName;
    SCIP_CONS *cons;
    for (int a = 0; a < data_.arcs.size(); ++a)
    {
        xVarName = "x[" + std::to_string(a) + ",0]";
        consName = "use[" + std::to_string(a) + ",0]";
        SCIP_CALL(SCIPcreateConsBasicLinear(scip_, &cons, consName.c_str(),
                                            0, nullptr, nullptr,
                                            0, SCIPinfinity(scip_)));
        SCIP_CALL(SCIPaddCoefLinear(scip_, cons, getVar(xVarName), -1.0));
        for (int t = 1; t < data_.timePeriods; ++t)
        {
            xVarName = "x[" + std::to_string(a) + "," + std::to_string(t) + "]";
            SCIP_CALL(SCIPaddCoefLinear(scip_, cons, getVar(xVarName), 1.0));
            SCIP_CALL(SCIPaddCons(scip_, cons));
            SCIP_CALL(SCIPreleaseCons(scip_, &cons));
            constraints_.push_back(cons);
            if (t < data_.timePeriods - 1)
            {
                consName = "use[" + std::to_string(a) + "," + std::to_string(t) + "]";
                SCIP_CALL(SCIPcreateConsBasicLinear(scip_, &cons, consName.c_str(),
                                                    0, nullptr, nullptr,
                                                    0, SCIPinfinity(scip_)));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, getVar(xVarName), -1.0));
            }
        }
        for (int t = 0; t < data_.timePeriods; ++t)
        {
            for (int m = 1; m < 3; ++m)
            {
                phiVarName = "Phi" + std::to_string(m) + "[" + std::to_string(a) + "," + std::to_string(t) + "]";
                xVarName = "x[" + std::to_string(a) + "," + std::to_string(t) + "]";
                betaVarName = "beta" + std::to_string(m) + "[" + std::to_string(t) + ",1]";

                SCIP_VAR *phiVar = getVar(phiVarName);
                SCIP_VAR *xVar = getVar(xVarName);
                SCIP_VAR *betaVar = getVar(betaVarName);

                consName = "McC1[" + std::to_string(a) + "," + std::to_string(t) + "]";
                SCIP_CALL(SCIPcreateConsBasicLinear(scip_, &cons, consName.c_str(),
                                                    0, nullptr, nullptr,
                                                    -SCIPinfinity(scip_), 0));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, phiVar, 1.0));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, xVar, -BETA_UB));
                SCIP_CALL(SCIPaddCons(scip_, cons));
                SCIP_CALL(SCIPreleaseCons(scip_, &cons));
                constraints_.push_back(cons);

                consName = "McC3[" + std::to_string(a) + "," + std::to_string(t) + "]";
                SCIP_CALL(SCIPcreateConsBasicLinear(scip_, &cons, consName.c_str(),
                                                    0, nullptr, nullptr,
                                                    -SCIPinfinity(scip_), 0));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, phiVar, 1.0));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, betaVar, -1.0));
                SCIP_CALL(SCIPaddCons(scip_, cons));
                SCIP_CALL(SCIPreleaseCons(scip_, &cons));
                constraints_.push_back(cons);

                consName = "McC4[" + std::to_string(a) + "," + std::to_string(t) + "]";
                SCIP_CALL(SCIPcreateConsBasicLinear(scip_, &cons, consName.c_str(),
                                                    0, nullptr, nullptr,
                                                    -BETA_UB, SCIPinfinity(scip_)));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, phiVar, 1.0));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, betaVar, -1.0));
                SCIP_CALL(SCIPaddCoefLinear(scip_, cons, xVar, -BETA_UB));
                SCIP_CALL(SCIPaddCons(scip_, cons));
                SCIP_CALL(SCIPreleaseCons(scip_, &cons));
                constraints_.push_back(cons);
            }
        }
    }
    return SCIP_OKAY;
}

SCIP_RETCODE ModelBuilder::solveModel()
{
    SCIP_CALL(SCIPsolve(scip_));
    SCIP_SOL *sol = SCIPgetBestSol(scip_);
    if (sol != nullptr)
    {
        std::cout << "Solution found:\n";
        for (auto var : variables_)
        {
            double val = SCIPgetSolVal(scip_, sol, var);
            if (val != 0)
            {
                std::cout << SCIPvarGetName(var) << " = " << val << "\n";
            }
        }
    }
    else
    {
        std::cout << "No solution found.\n";
    }
    return SCIP_OKAY;
}

void ModelBuilder::printObjFun()
{
    printf("Objective: ");
    for (auto var : variables_)
    {
        SCIP_Real obj = SCIPvarGetObj(var);
        if (SCIPisZero(scip_, obj))
            continue;

        const char *name = SCIPvarGetName(var);
        printf("%+g%s ", obj, name);
    }
    printf("\n");
}

SCIP_VAR *ModelBuilder::getVar(const std::string &name)
{
    for (auto var : variables_)
    {
        if (SCIPvarGetName(var) == name)
            return var;
    }
    return nullptr;
}

SCIP_CONS *ModelBuilder::getCons(const std::string &name)
{
    for (auto cons : constraints_)
    {
        if (SCIPconsGetName(cons) == name)
            return cons;
    }
    return nullptr;
}

std::vector<SCIP_VAR *> ModelBuilder::getAllVars()
{
    return variables_;
}

std::vector<SCIP_CONS *> ModelBuilder::getAllCons()
{
    return constraints_;
}
