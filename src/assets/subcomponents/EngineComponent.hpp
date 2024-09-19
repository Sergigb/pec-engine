#ifndef ENGINE_COMPONENT_HPP
#define ENGINE_COMPONENT_HPP

#include <vector>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

class BasePart;


// propellant required by the engine
struct required_propellant{
    double mass_flow_rate; // flow rate at 100% throttle
    std::uint32_t resource_id; // resource id, the ids can be gotten by hasing its name.

    required_propellant(double flow_rate, std::uint32_t resource){
        mass_flow_rate = flow_rate;
        resource_id = resource;
    }
};

#define ENGINE_OFF 1
#define ENGINE_ON 2
#define ENGINE_DAMAGED 3

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
        // thrust orientation, already in matrix form
        btVector3 m_local_orientation;
        // deflection angles
        double m_max_angle_yaw, m_max_angle_pitch;
        // yaw, pitch controls
        bool m_yaw, m_pitch;
        // maximum average thrust of the engine
        double m_max_avg_thrust;
        // required propellants by this engine
        std::vector<struct required_propellant> m_propellants;

        // engine status
        int m_status;

        BasePart* m_parent_part;

    public:
        /*
         * Constructor
         *
         * @parent_part: part that contains this engine.
         * @origin: local origin (on the part) of the thrust.
         * @orientation: local orientation (on the part) of the thrust. Most likely you want to set
         * it to (0.0, -1.0, 0.0). Should be a unit vector, but will be normalised anyways.
         */
        EngineComponent(BasePart* parent_part, const btVector3& origin,
                        const btVector3& orientation, double max_avg_thrust);

        EngineComponent();

        /*
         * Adds a required propellant by the engine.
         *
         * @flow_rate: mass flow rate of the propellant at 100% throttle.
         * @resource: resource id of the propellant.
         */
        void addPropellant(double flow_rate, std::uint32_t resource);

        /*
         * Returns the vector with the required engine propellants.
         */
        const std::vector<struct required_propellant>& getPropellants() const;

        /*
         * Starts the engine, sets m_status to ENGINE_ON.
         */
        void startEngine();

        /*
         * Gets the status of the engine.
         */
        int getEngineStatus() const;

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
};


#endif
