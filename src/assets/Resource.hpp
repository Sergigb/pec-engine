#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <string>
#include <cstdint>


#define RESOURCE_TYPE_LIQUID 1
#define RESOURCE_TYPE_SOLID 2
#define RESOURCE_TYPE_FUEL_LIQUID 4
#define RESOURCE_TYPE_FUEL_SOLID 8
#define RESOURCE_TYPE_FUEL_GAS 16
#define RESOURCE_TYPE_OXIDIZER 32


#define RESOURCE_STATE_SOLID 1
#define RESOURCE_STATE_LIQUID 2
#define RESOURCE_STATE_GAS 3


/*
 * Resource class.
 */
class Resource{
    private:
        std::string m_resource_name;
        std::string m_resource_fancy_name;
        char m_resource_type;
        char m_resource_state;
        double m_density; // kg/M3
        double m_temperature; // temperature at which it has to be stored in this state
        std::uint32_t m_resource_id;
    public:
        /*
         * Constructor.
         *
         * @name: simple name of the resource.
         * @fancy_name: name of the resource, it can have unicode chars. The encoding of the
         * @strings sucks right now.
         * @rtype: type of resource, defined in the macros above (RESOURCE_TYPE_*).
         * @rstate: physical state of the resource, solid, liquid or gas. Use the macros from
         * above (RESOURCE_STATE_*).
         * @density: density of the resource, in kg/M^3.
         * @temperature: temperature at which it has to be stored in this state.
         */
        Resource(const std::string& name, const std::string& fancy_name, char rtype, char rstate, double density, double temperature);

        ~Resource();

        /*
         * Sets the ID of the resource.
         */
        void setId(std::uint32_t id);

        /*
         * Returns the type of the resource (macros RESOURCE_TYPE_*).
         */
        char getType() const;

        /*
         * Returns the state of the resource (macros RESOURCE_STATE_*).
         */
        char getState() const;

        /*
         * Returns the density of the resource, in kg/M^3.
         */
        double getDensity() const;

        /*
         * Returns the temperature of the resource.
         */
        double getTemperature() const;

        /*
         * Returns a constant reference to the name of the resource.
         */
        const std::string& getName() const;

        /*
         * Returns a constant reference to the fancy name of the resource.
         */
        const std::string& getFancyName() const;

        /*
         * Returns the ID of the resource.
         */
        std::uint32_t getId() const;
};


#endif