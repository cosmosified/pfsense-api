<?php
require_once("api/framework/APIBaseModel.inc");
require_once("api/framework/APIResponse.inc");
require_once("api/api_models/APISystemDNS.inc");


class APISystemDNSModify extends APIBaseModel {
    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->methods = ["POST"];
        $this->privileges = ["page-all", "page-system"];
        $this->change_note = "Modified system DNS servers via API";
    }

    public function action() {
        $this->write_config();    // Apply our configuration change
        // Update a slew of backend services
        system_resolvconf_generate();
        if (isset($this->config['dnsmasq']['enable'])) {
            services_dnsmasq_configure();
        } elseif (isset($this->config['unbound']['enable'])) {
            services_unbound_configure();
        }

        // Reload DNS services and firewall filter
        send_event("service reload dns");
        filter_configure();
        return APIResponse\get(0, (new APISystemDNS())->action()["data"]);
    }

    public function validate_payload() {
        if (isset($this->initial_data['dnsserver'])) {
            $this->initial_data["dnsserver"] = $this->initial_data['dnsserver'];
            // If value is not an array, convert it
            if (!is_array($this->initial_data["dnsserver"])) {
                $this->initial_data["dnsserver"] = array($this->initial_data["dnsserver"]);
            }
            // Loop through our DNS servers and check that entry is valid
            foreach ($this->initial_data["dnsserver"] as $ds) {
                // Check if our DNS server is valid
                if (!is_ipaddrv4($ds) and !is_ipaddrv6($ds)) {
                    $this->errors[] = APIResponse\get(1007);
                }
            }
            // Add our system DNS values to validated data
            $this->config["system"]["dnsserver"] = $this->initial_data["dnsserver"];
        }
        if ($this->initial_data['dnsallowoverride'] === true) {
            $this->config["system"]["dnsallowoverride"] = "";
        } elseif ($this->initial_data['dnsallowoverride'] === false) {
            unset($this->config["system"]["dnsallowoverride"]);
        }
        if ($this->initial_data['dnslocalhost'] === true) {
            $this->config["system"]["dnslocalhost"] = "";
        } elseif ($this->initial_data['dnslocalhost'] === false) {
            unset($this->config["system"]["dnslocalhost"]);
        }
    }
}