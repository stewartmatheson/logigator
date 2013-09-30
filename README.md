#Logigator

_"Log aggregation with teeth"_ 

Logigator is real-time log aggregator for distributed systems. Logigator takes “unix flavor” system and application logs and renders them in the browser for easy reading. Logodile replaces the need to have console windows open with “tail -f”.

##The client
 * Is installed on n servers
 * Reads logs in real time and calls home to the server when logs are updated (sockets?)
 * Can read n logs at a given time
 * Does things the unix way
   * Runs as a daemon
   * Writes a pid /var/run/logodile.pid
   * Is configurable /etc/logodile.conf
   * Logs itself /var/log/logodile.log
 * Is very small
 * Is highly portable
 * Has a small memory footprint
 * Installed via a package???

##The Server
 * Is a RESTful API 
 * Is backed by no-sql eg mongo or couch
 * Accepts logs from n clients
 * Persists logs in on standard way no matter where they come from
 * Exposes API’s to fetch logs

##The Interface
 * Is written in HTML5 and Javascript (HTML5BP?)
 * Renders logs in real time (i.e no page refresh) (web sockets?)
 * Can filter by…
 * Host name
 * Log file
 * Date
 * Displays logs in a human readable way
 * Has a cool logo and looks good in a screen shot on github
 * Responsive design

