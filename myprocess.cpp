#include<bits/stdc++.h>
#include "json.hpp"
#include<cmath>
using json = nlohmann::json;
using namespace std;

struct node {
    int id;
    string name;
    double lat,lon;
    bool accessible;
    string type;
};

struct edge {
    int from,to;
    double distance;
    double time;
    bool accessible;
    string name;
};



double haversine(double lat1,double lon1,double lat2,double lon2) {
    double r=6371000;
    double phi1=lat1*M_PI/180.0;//converting latitude and longitude into radians
    double phi2=lat2*M_PI/180.0;
    double delphi=(lat2-lat1)*M_PI/180.0;
    double dellambda=(lon2-lon1)*M_PI/180.0;

    double a=sin(delphi/2)*sin(delphi/2)+cos(phi1)*cos(phi2)*sin(dellambda/2)*sin(dellambda/2);
    double c=2*atan2(sqrt(a),sqrt(1-a));
    return r*c;
}

double find_existing_node(double lat,double lon,map<int,node> &nodes,double threshold) {
    for(auto& pair : nodes) {
        int id=pair.first;
        node n=pair.second;
        double d=haversine(lat,lon,n.lat,n.lon);
        if(d<=threshold) return id;
    }
    return -1;
}

double find_nearest_node(double lat,double lon,map<int,node> &nodes,int exclude_id=-1,set<string> exclude_type={}) {
    double min_dis=1e18;
    int nearest_id=-1;
    for(auto& pair : nodes) {
        int id=pair.first;
        node n=pair.second;
        if(exclude_id!=-1&&id==exclude_id) {
            continue;
        }
        if(exclude_type.count(n.type)) continue;

        double d =haversine(lat,lon,n.lat,n.lon);
        if(d<min_dis) {
            min_dis=d;
            nearest_id=id;
        }
    }
    return nearest_id;
}

void connect_node_to_nearest_node(int node_id,double lat,double lon,map<int,node> &nodes,vector<edge> &edges,bool accessible,set<string> exclude_type={}) {
    if(!nodes.empty()) {
        int nearest_node_id=find_nearest_node(lat,lon,nodes,node_id,exclude_type);
        if(nearest_node_id!=-1&& nearest_node_id!= node_id) {
            node from =nodes[node_id];
            node to =nodes[nearest_node_id];

            double d=haversine(lat,lon,to.lat,to.lon);
            double time = d/1.4; // speed of wallking is 1.4m/s

            edge e1={node_id,nearest_node_id,d,time,accessible,""};
            edge e2={nearest_node_id,node_id,d,time,accessible,""};
            edges.push_back(e1);
            edges.push_back(e2);
        }
    }
}

int main() {
    ifstream f("export.geojson");
    json data;
    f >> data;


    map<int,node> nodes;
    vector<edge> edges;
    int node_id_counter=1;
    double threshold=1.0;

    vector<json> line_features,polygon_features;

    for(auto feature : data["features"]) {
        string type=feature["geometry"]["type"];
        if(type=="LineString") line_features.push_back(feature); 
        else if(type=="Polygon") polygon_features.push_back(feature);
    }

    for(auto feature : line_features) {
        bool accessible =true;
        vector<int> nodes_in_way;
        for(auto coord : feature["geometry"]["coordinates"])  {
            double lon =coord[0],lat=coord[1];
            int existing = find_existing_node(lat,lon,nodes,threshold);
            int node_id;
            if(existing!=-1) {
                node_id=existing;
            } else {
                node_id=node_id_counter;
                node_id_counter++;
                nodes[node_id]={node_id,"",lat,lon,accessible,"path"};
            }
            nodes_in_way.push_back(node_id);
        }

        for(int i=0;i+1<nodes_in_way.size();i++) {
            node from =nodes[nodes_in_way[i]];
            node to =nodes[nodes_in_way[i+1]];
            double d=haversine(from.lat,from.lon,to.lat,to.lon);
            double t= d/1.4;
            edges.push_back({from.id,to.id,d,t,accessible,""});
            edges.push_back({to.id,from.id,d,t,accessible,""});

        }
    }

    for(auto feature : polygon_features) {
        string name=feature["properties"].value("name","");
        if(name.empty() ) continue;
        bool accessible =true;
        vector<int> nodes_in_buildings;
        for(auto coord : feature["geometry"]["coordinates"][0]) {
            double lon=coord[0],lat=coord[1];
            int existing=find_existing_node(lat,lon,nodes,threshold);
            int node_id;
            if(existing!=-1) {
                node_id=existing;
                if(nodes[node_id].name.empty()) nodes[node_id].name=name;
            } else {
                node_id=node_id_counter;
                node_id_counter++;
                nodes[node_id]={node_id,name,lat,lon,accessible,"building"};
            }
            nodes_in_buildings.push_back(node_id);
            connect_node_to_nearest_node(node_id,lat,lon,nodes,edges,accessible,{"building"});

        }

        for(int i=0;i+1<nodes_in_buildings.size();i++) {
            node from =nodes[nodes_in_buildings[i]];
            node to =nodes[nodes_in_buildings[(i+1)%nodes_in_buildings.size()]];
            double d= haversine(from.lat,from.lon,to.lat,to.lon);
            double t= d/1.4;
            edges.push_back({from.id,to.id,d,t,accessible,""});
            edges.push_back({to.id,from.id,d,t,accessible,""});

        }

    }

    json output;
    output["nodes"]=json::array();
    for(auto& pair : nodes) {
        int id=pair.first;
        node n=pair.second;
        output["nodes"].push_back({
            {"id",n.id},
            {"name",n.name},
            {"lat",n.lat},
            {"lon",n.lon},
            {"accessible",n.accessible},
            {"type",n.type}
        });
    }
    output["edges"]=json::array();
    for(auto e :edges) {
        output["edges"].push_back({
            {"from",e.from},
            {"to",e.to},
            {"distance",e.distance},
            {"time",e.time},
            {"accessible",e.accessible},
            {"name",e.name}
        });
    }

    ofstream out("iitkgp.json");
    out<<output.dump(2);
    cout<<"data extraction and conversion completed"<<endl;
}


