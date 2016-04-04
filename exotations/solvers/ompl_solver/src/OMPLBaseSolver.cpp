/*
 * OMPLBaseSolver.cpp
 *
 *  Created on: 22 Feb 2016
 *      Author: yiming
 */

#include "ompl_solver/OMPLBaseSolver.h"

namespace exotica
{
  OMPLBaseSolver::OMPLBaseSolver(const std::string planner_name)
      : timeout_(60.0), finishedSolving_(false), planner_name_(planner_name)
  {

  }
  OMPLBaseSolver::~OMPLBaseSolver()
  {

  }

  EReturn OMPLBaseSolver::initialiseBaseSolver(tinyxml2::XMLHandle & handle,
      const Server_ptr &server)
  {
    server_ = server;
    registerDefaultPlanners();
    return initialiseSolver(handle);
  }

  void OMPLBaseSolver::recordData()
  {
    ompl::base::PlannerData data(ompl_simple_setup_->getSpaceInformation());
    ompl_simple_setup_->getPlanner()->getPlannerData(data);
    int cnt =
        prob_->getScenes().begin()->second->getCollisionScene()->stateCheckCnt_;
    prob_->getScenes().begin()->second->getCollisionScene()->stateCheckCnt_ = 0;
  }

  double OMPLBaseSolver::getPlanningTime()
  {
    return planning_time_.toSec();
  }

  bool OMPLBaseSolver::preSolve()
  {
    // clear previously computed solutions
    ompl_simple_setup_->getProblemDefinition()->clearSolutionPaths();
    const ob::PlannerPtr planner = ompl_simple_setup_->getPlanner();
    if (planner)
    {
      planner->clear();
    }
    else
    {
      ERROR("Planner not found");
      return false;
    }
    ompl_simple_setup_->getSpaceInformation()->getMotionValidator()->resetMotionCounter();
    ompl_simple_setup_->getPlanner()->setProblemDefinition(
        ompl_simple_setup_->getProblemDefinition());
    return true;
  }

  bool OMPLBaseSolver::postSolve()
  {
    ompl_simple_setup_->clearStartStates();
    int v =
        ompl_simple_setup_->getSpaceInformation()->getMotionValidator()->getValidMotionCount();
    int iv =
        ompl_simple_setup_->getSpaceInformation()->getMotionValidator()->getInvalidMotionCount();
    logDebug("There were %d valid motions and %d invalid motions.", v, iv);

    if (ompl_simple_setup_->getProblemDefinition()->hasApproximateSolution())
    logWarn("Computed solution is approximate");
    return true;
  }

  void OMPLBaseSolver::registerTerminationCondition(
      const ob::PlannerTerminationCondition &ptc)
  {
    boost::mutex::scoped_lock slock(ptc_lock_);
    ptc_ = &ptc;
  }

  void OMPLBaseSolver::unregisterTerminationCondition()
  {
    boost::mutex::scoped_lock slock(ptc_lock_);
    ptc_ = NULL;
  }

  const boost::shared_ptr<og::SimpleSetup> OMPLBaseSolver::getOMPLSimpleSetup() const
  {
    return ompl_simple_setup_;
  }
}