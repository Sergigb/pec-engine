#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <string>
#include <cstdint>


#define RESOURCE_TYPE_LIQUID 1
#define RESOURCE_TYPE_SOLID 2
#define RESOURCE_TYPE_FUEL_LIQUID 4
#define RESOURCE_TYPE_FUEL_SOLID 8
#define RESOURCE_TYPE_FUEL_GAS
#define RESOURCE_TYPE_OXIDIZER 16


#define RESOURCE_STATE_SOLID 1
#define RESOURCE_STATE_LIQUID 2
#define RESOURCE_STATE_GAS 3


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
        Resource(const std::string& name, const std::string& fancy_name, char rtype, char rstate, double density, double temperature);
        ~Resource();
        void setId(std::uint32_t id);

        char getType() const;
        char getState() const;
        double getDensity() const;
        double getTemperature() const;
        void getName(std::string& name) const;
        void getFancyName(std::string& fancy_name) const;
};


#endif