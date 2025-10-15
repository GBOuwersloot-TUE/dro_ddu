#pragma once
#include "scip/scip.h"
#include "model_data.hpp"
#include <vector>

class ModelBuilder
{
public:
    ModelBuilder(const ModelData &data);
    ~ModelBuilder();
    SCIP_RETCODE constructModel();
    SCIP_RETCODE deconstructModel();
    SCIP_RETCODE buildModel();
    SCIP_RETCODE addVars();
    SCIP_RETCODE addVar(const std::string &varName, double lb, double ub, double obj, SCIP_VARTYPE varType);
    SCIP_RETCODE addConstraints();
    SCIP_RETCODE addCons(const std::string &consName);
    SCIP_RETCODE solveModel();
    void printObjFun();
    SCIP_VAR *getVar(const std::string &name);
    SCIP_CONS *getCons(const std::string &name);
    std::vector<SCIP_VAR *> getAllVars();
    std::vector<SCIP_CONS *> getAllCons();

private:
    SCIP *scip_;
    const ModelData &data_;
    std::vector<SCIP_VAR *> variables_;
    std::vector<SCIP_CONS *> constraints_;
};