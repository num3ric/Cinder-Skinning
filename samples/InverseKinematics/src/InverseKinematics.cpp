//
//  InverseKinematics.cpp
//  EigenTest
//
//  Created by Éric Renaud-Houde on 2014-05-15.
//
//

#include "InverseKinematics.h"


using namespace ik;
using namespace ci;

namespace Eigen {
	Eigen::Vector3f fromCi( const vec3& v ) {
		return Eigen::Vector3f( v.x, v.y, v.z );
	}
	
	vec3 toCi( const Eigen::Vector3f& v ) {
		return vec3( v[0], v[1], v[2] );
	}
}

Solver::Solver()
: mStepsize( 0.00001 )
, mLambda( 100.0 )
, mMaxStep( 65.0 )
, mUseDLS( true )
, mUseAlternateJacobian( true ) {
	
}

void Solver::setGoal( const vec3& goal )
{
	mGoal = Eigen::fromCi( goal );
}

ci::vec3 Solver::getGoal() const
{
	return Eigen::toCi( mGoal );
}

ci::vec3 Solver::getClampedGoal() const
{
	return Eigen::toCi( mClampedGoal );
}

void Solver::solve( const std::vector<model::NodeRef>& chain, const model::NodeRef& endEffector )
{
	double radius = 0;
	for( auto &joint : chain ) {
		radius += length( joint->getRelativePosition() );
	}
	radius += length( endEffector->getRelativePosition() );
	
	mClampedGoal = clampMag( mGoal, radius );
	
	if( distance2( endEffector->getAbsolutePosition(), Eigen::toCi( mClampedGoal ) ) < 0.0001f )
		return;
	
	size_t dim = chain.size();
	// Compute the jacobian
	Eigen::MatrixXf jacobian( 3, dim );
	jacobian.setZero();
	for( int i = dim - 1; i >= 0; --i ) {
		Eigen::Vector3f target = ( mUseAlternateJacobian ) ? mClampedGoal : Eigen::fromCi( endEffector->getAbsolutePosition() );
		Eigen::Vector3f diff = target - Eigen::fromCi( chain.at( i )->getAbsolutePosition() );
		vec3 axis;
		// Consider root joints to be 3-DOF quaternion joints. TODO: Support different DOFs
		axis = computeAxis( endEffector->getAbsolutePosition(), Eigen::toCi( mClampedGoal ), chain.at( i )->getAbsolutePosition() );
		mAxes[chain.at( i )] = axis;
		jacobian.col( i ) = diff.cross( Eigen::fromCi( axis ) );
	}
	
	// Compute (clamped) V
	auto V = clampMag( mClampedGoal - Eigen::fromCi( endEffector->getAbsolutePosition() ), mMaxStep );
	
	Eigen::VectorXf theta_dot( dim );
	if( mUseDLS ) {
		//Solve via damped least-squares method (SVD decomposition)
		theta_dot = dlsSVDMethod( jacobian, V, mLambda );
	}
	else {
		// Solve via Jacobian transpose method
		theta_dot = mStepsize * jacobian.transpose() * V;
	}
	
	// Update system with the computed rotation updates
	for( int i = dim - 1; i >= 0; --i ) {
		quat rot = chain.at( i )->getAbsoluteRotation();
		quat step = angleAxis( -theta_dot[i], mAxes.at( chain.at( i ) ) );
		chain.at( i )->setAbsoluteRotation( step * rot );
	}
}

Eigen::Vector3f Solver::clampMag( Eigen::Vector3f v, double maxStep ) {
	if( v.norm() < maxStep ) {
		return v;
	}
	else {
		return maxStep * v.normalized();
	}
}

Eigen::VectorXf Solver::dlsSVDMethod( const Eigen::MatrixXf & jacobian, const Eigen::VectorXf & diff, double lambda ) {
	size_t dim = jacobian.rows();
	auto Jt = jacobian.transpose();
	Eigen::JacobiSVD<Eigen::MatrixXf> svd( jacobian * Jt + lambda * lambda * Eigen::MatrixXf::Identity( dim, dim ),
										  Eigen::ComputeThinU | Eigen::ComputeThinV );
	return Jt * svd.solve( diff );
}

vec3 Solver::computeAxis( const vec3& endEffector, const vec3& goal, const vec3& joint ) {
	return normalize( cross( endEffector - joint , goal - joint ) );
}

