'use strict';
const options = require('./options')();
const services = {}

/**
 * Gets or creates a service. The available services are:
 * @param {string} name - Service name
 * @return {*} Initialized service module.
 */
module.exports = (serviceName) => {
    if (!services[serviceName])
        services[serviceName] = require('./service/' + serviceName)(options[serviceName] || {});
    return services[serviceName];
}
