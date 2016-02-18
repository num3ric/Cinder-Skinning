#pragma once

#include "cinder/Vector.h"

#if defined( CINDER_MSW )
#include <Eigen/Dense>
#else
#include "Eigen/Dense"
#endif
#include "model/Node.h"

#include <unordered_map>

namespace ik {
	
	class Solver {
	public:
		Solver();
		
		void		setGoal( const glm::vec3& goal );
		glm::vec3	getGoal() const;
		glm::vec3	getClampedGoal() const;
		
		void solve( const std::vector<model::NodeRef>& chain, const model::NodeRef& endEffector );
		
		bool mUseDLS, mUseAlternateJacobian;
		
		double mStepsize;
		double mLambda;
		double mMaxStep;
	private:
		Eigen::Vector3f clampMag( Eigen::Vector3f v, double maxStep );
		
		Eigen::VectorXf dlsSVDMethod( const Eigen::MatrixXf & jacobian, const Eigen::VectorXf & diff, double lambda );
		
		glm::vec3 computeAxis( const glm::vec3& endEffector, const glm::vec3& goal, const glm::vec3& joint );
		
		Eigen::Vector3f mGoal, mClampedGoal;
		
		std::unordered_map<model::NodeRef, glm::vec3> mAxes;
	};
	
}