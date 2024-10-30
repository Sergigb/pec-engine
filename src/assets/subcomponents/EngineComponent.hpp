#ifndef ENGINE_COMPONENT_HPP
#define ENGINE_COMPONENT_HPP

#include <vector>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

class BasePart;


// propellant required by the engine
struct required_propellant{
    double max_flow_rate; // flow rate at 100% throttle
    double current_flow_rate;  // current flow rate, is used by some function below to compute the thrust
    std::uint32_t resource_id; // resource id, the ids can be gotten by hasing its name.

    required_propellant(double mfr, std::uint32_t resource){
        max_flow_rate = mfr;
        resource_id = resource;
        current_flow_rate = 0.;
    }
};

#define ENGINE_STATUS_OFF 1
#define ENGINE_STATUS_ON 2
#define ENGINE_STATUS_DAMAGED 3

/*
 * Represents a base engine, a part can have none, one or multiple engines. Every engine has its
 * own local origin (where the force is applied), local orientation, thrust parameters etc. Any
 * part that contains an engine should use this class or make a derived one. This will be useful to
 * standarize an interface for engines. If a part has an engine and does use this class nor
 * register it (include it in m_engine_list) it can not be seen by things like PDI controllers,
 * automatic pilots and such.
 */
class EngineComponent{
    private:
        // local origin
        btVector3 m_local_origin;
        // thrust orientation
        btVector3 m_local_orientation;
        // deflection angles
        double m_max_angle_yaw, m_max_angle_pitch;
        // yaw, pitch controls
        bool m_yaw, m_pitch;
        // maximum average thrust of the engine
        double m_max_avg_thrust;
        // current throttle of the engine (0.0 to 1.0)
        double m_throttle;
        // required propellants by this engine
        std::vector<struct required_propellant> m_propellants;
        // if true, can be stopped
        bool m_can_be_stopped;

        // engine status
        int m_status;

        BasePart* m_owner_part;

    public:
        /*
         * Constructor
         *
         * @owner_part: part that contains this engine.
         * @origin: local origin (on the part) of the thrust.
         * @orientation: local orientation (on the part) of the thrust. Most likely you want to set
         * it to (0.0, -1.0, 0.0). Should be a unit vector, but will be normalised anyways.
         */
        EngineComponent(BasePart* owner_part, const btVector3& origin,
                        const btVector3& orientation, double max_avg_thrust);

        EngineComponent();

        /*
         * Adds a required propellant by the engine.
         *
         * @resource: resource id of the propellant.
         * @flow_rate: mass flow rate of the propellant at 100% throttle.
         */
        void addPropellant(std::uint32_t resource, double flow_rate);

        /*
         * Returns the vector with the required engine propellants.
         */
        const std::vector<struct required_propellant>& getPropellants() const;

        /*
         * Starts the engine, sets m_status to ENGINE_STATUS_ON.
         */
        void startEngine();

        /*
         * Stp`s the engine, sets m_status to ENGINE_STATUS_OFF. Might not take effect if the
         * engine can not be shut down.
         */
        void stopEngine();

        /*
         * Gets the status of the engine.
         */
        int getStatus() const;

        /*
         * Returns the local origin of the thrust with respect its part. The force/thrust should be
         * applied in this point.
         */
        const btVector3& getThrustOrigin() const;

        /*
         * Returns the local UNDEFLECTED orientation of the thrust in vector form. This vector is
         * notmalized, and does not represent the final thrust.
         */
        const btVector3& getThrustOrientation() const;

        /*
         * Returns a basis transform according to the yaw and pitch values of the vessel.
         */
        const btMatrix3x3 getThrustDeflectionBasis() const;

        /*
         * Returns the deflected orientation of the thrust. This value does not represent the final
         * thrust value, as it is normalized.
         */
        const btVector3 getDeflectedThrust() const;

        /*
         * Sets the thrust deflection/vectoring parameters.
         *
         * @yaw: if true, the engine can deflect in the x axis.
         * @pitch: if true, the engine can deflect in the z axis.
         * @max_angle_yaw: max deflection angle in the x axis. If yaw is false, this parameter is
         * not used.
         * @max_angle_pitch: max deflection angle in the x axis. If pitch is false, this parameter
         * is not used.
         */
        void setDeflectionParams(bool yaw, bool pitch, double max_angle_yaw,
                                             double max_angle_pitch);

        /*
         * Gets the thrust deflection/vectoring parameters.
         *
         * @yaw: if true, the engine can deflect in the x axis.
         * @pitch: if true, the engine can deflect in the z axis.
         * @max_angle_yaw: max deflection angle in the x axis
         * @max_angle_pitch: max deflection angle in the x axis
         */
        void getDeflectionParams(bool& yaw, bool& pitch, double& max_angle_yaw,
                                       double& max_angle_pitch) const;

        /*
         * Returns the maximum thrust of the engine at 100% throttle (wether this thrust can be
         * pushed further depends on your simulation of the engine). Does not represent the current
         * thrust, but the theoretical maximum thrust.
         */
        double getMaxAvgThrust() const;

        /*
         * Sets wether the engine can be stopped or not.
         *
         * @can_be_stopped: sets the property to true/false.
         */
        void setStop(bool can_be_stopped);

        /*
         * Sets the engine throttle.
         *
         * @throttle: throttle value, from 0.0 to 1.0.
         */
        void setThrottle(double throttle);

        /*
         * Gets the current throttle.
         */
        double getThrottle() const;

        /*
         * Updates the amount of resources that flow into the engine. This function requests
         * resources to the parent part of the owner part, but it could also request resources to
         * the owner engine in a derived class. Depends on the requires resources and the current
         * throttle.
         */
        void updateResourcesFlow();

        /*
         * Sets the owner part of this engine.
         *
         * @owner_part: part that contains this engine.
         */
        void setOwner(BasePart* owner_part);
};


#endif
