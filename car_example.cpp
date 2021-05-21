#include <algorithm>
#include <iostream>
#include <fstream>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>

#include <signal.h>
#include <string.h>


using namespace std;
using namespace Pistache;

string stateful(string param){
    cout<<"intra";
    ifstream in(param);
    string date, cdate;
    in>>date;
    cdate = date;
    return cdate;
    }

void stateful2(string date, string param){
    ofstream out(param);
    out<<date;
}

// General advice: pay atetntion to the namespaces that you use in various contexts. Could prevent headaches.

// This is just a helper function to preety-print the Cookies that one of the enpoints shall receive.
void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

// Some generic namespace, with a simple function we could use to test the creation of the endpoints.
namespace Generic {

    void handleReady(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "1");
    }

}

// Definition of the CarEnpoint class 
class CarEndpoint {
public:
    explicit CarEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    // Initialization of the server. Additional options can be provided here
    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(static_cast<int>(thr));
        httpEndpoint->init(opts);
        // Server routes are loaded up
        setupRoutes();
    }

    // Server is started threaded.  
    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    // When signaled server shuts down
    void stop(){
        httpEndpoint->shutdown();
    }
    

private:
    void setupRoutes() {
        using namespace Rest;
        // Defining various endpoints
        // Generally say that when http://localhost:9080/ready is called, the handleReady function should be called. 
        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        Routes::Get(router, "/auth", Routes::bind(&CarEndpoint::doAuth, this));
        Routes::Post(router, "/settings/:settingName/:value?", Routes::bind(&CarEndpoint::setSetting, this));
        Routes::Get(router, "/settings/:settingName/", Routes::bind(&CarEndpoint::getSetting, this));
    }

    
    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        // Function that prints cookies
        printCookies(request);
        // In the response object, it adds a cookie regarding the communications language.
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        // Send the response
        response.send(Http::Code::Ok);
    }

    // Endpoint to configure one of the Car's settings.
    void setSetting(const Rest::Request& request, Http::ResponseWriter response){
        // You don't know what the parameter content that you receive is, but you should
        // try to cast it to some data structure. Here, I cast the settingName to string.
        auto settingName = request.param(":settingName").as<std::string>();

        // This is a guard that prevents editing the same value by two concurent threads. 
        Guard guard(carLock);

        
        string val = "";
        if (request.hasParam(":value")) {
            auto value = request.param(":value");
            val = value.as<string>();
        }

        // Setting the car's setting to value
        int setResponse = v_car.set(settingName, val);

        // Sending some confirmation or error response.
        if (setResponse == 1) {
            response.send(Http::Code::Ok, settingName + " was set to " + val);
        }
        else {
            response.send(Http::Code::Not_Found, settingName + " was not found and or '" + val + "' was not a valid value ");
        }

    }

    // Setting to get the settings value of one of the configurations of the Car
    void getSetting(const Rest::Request& request, Http::ResponseWriter response){
        auto settingName = request.param(":settingName").as<std::string>();

        Guard guard(carLock);
        string valueSetting = v_car.get(settingName);

        if (valueSetting != "") {

            // In this response I also add a couple of headers, describing the server that sent this response, and the way the content is formatted.
            using namespace Http;
            response.headers()
                        .add<Header::Server>("pistache/0.1")
                        .add<Header::ContentType>(MIME(Text, Plain));

            response.send(Http::Code::Ok, settingName + " is " + valueSetting);
        }
        else {
            response.send(Http::Code::Not_Found, settingName + " was not found");
        }
    }

    // Defining the class of the Car. It should model the entire configuration of the Car
    class Car {
    public:
        explicit Car(){ }

        // Setting the value for one of the settings.
        int set(std::string name, std::string value){
            if(name == "temperature"){
                temperature.name = name;
                temperature.value = value;
                stateful2(value, "stateful.txt");
                return 1;
            }
            if (name == "oprire"){
                //oprire.value = value;
                std::string all_km_str = stateful("stateful_current.txt");
                int all_km = stoi(all_km_str); 
                int new_km = rand()%999+2;
                //int new_km = stoi(oprire.value);
                new_km = new_km + all_km;
                std::string new_km_str = std::to_string(new_km);
                stateful2(new_km_str,"stateful_current");
                return 1;
            }
            if(name == "revision_done"){
                string last_revision = stateful("stateful_current_km.txt");
                stateful2(last_revision , "stateful_verified_km.txt");
                return 1;
            }
            if(name == "senzor") {
              distance.value = value;
              stateful2(value, "stateful_senzor.txt");
              return 1;
            }
            return 0;
        }

        // Getter
        string get(std::string name){
            if (name == "temperature"){
                temperature.value = stateful("stateful.txt");
                return temperature.value;
            }
            else if (name == "revision"){
                std::string last_revision_str = stateful("stateful_verified_km.txt");
                int last_revision = stoi(last_revision_str);
                int all_km = stoi(stateful("stateful_current_km.txt"));

                if(all_km - last_revision >= 10000){
                    return "Time for revision!";
                }
                else { return "Keep rolling!";}
            }
            else if(name == "senzor") {
                distance.value = stateful("stateful_senzor.txt");
                string brake = "false";
                if(stoi(distance.value) < 3){
                    brake = "true";
                }
                return brake;
                }
            else{
            return "";
            }
        }



    private:
        // Defining and instantiating settings.
        struct intSetting{
        
            std::string name;
            string value;
        }temperature, distance;
    };

    // Create the lock which prevents concurrent editing of the same variable
    using Lock = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock carLock;

    // Instance of the car model
    Car v_car;

    // Defining the httpEndpoint and a router.
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {


    // This code is needed for gracefull shutdown of the server when no longer needed.
    sigset_t signals;
    if (sigemptyset(&signals) != 0
            || sigaddset(&signals, SIGTERM) != 0
            || sigaddset(&signals, SIGINT) != 0
            || sigaddset(&signals, SIGHUP) != 0
            || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {
        perror("install signal handler failed");
        return 1;
    }

    // Set a port on which your server to communicate
    Port port(9080);

    // Number of threads used by the server
    int thr = 2;

    if (argc >= 2) {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3)
            thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    // Instance of the class that defines what the server can do.
    CarEndpoint stats(addr);

    // Initialize and start the server
    stats.init(thr);
    stats.start();


    // Code that waits for the shutdown sinal for the server
    int signal = 0;
    int status = sigwait(&signals, &signal);
    if (status == 0)
    {
        std::cout << "received signal " << signal << std::endl;
    }
    else
    {
        std::cerr << "sigwait returns " << status << std::endl;
    }

    stats.stop();
}