#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <stack>
#include <queue>
#include <functional>
#include "implot.h"
#include <ctime>
#include <cstdio>
#include <stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

//FOR TESTING ONLY
//const int ADDITIONAL_NODES = 1; // Change to 10, 100, 1000, 10000
//FOR TESTING ONLY 

using namespace std;

// Algorithm graph
unordered_map<string, vector<string>> sccGraph;
unordered_map<string, vector<pair<string, double>>> dijkstraGraph;

// Results
unordered_map<string, double> dijkstraResult;
vector<vector<string>> tarjanResult;
vector<vector<string>> pathBasedResult;
double tarjanTime = 0.0;
double pathBasedTime = 0.0;

// Texture
GLuint texture = 0; 
int imgWidth, imgHeight, imgChannels;

void AddEdge(const string& from, const string& to, double distance) {
    sccGraph[from].push_back(to);
    if (sccGraph.find(to) == sccGraph.end()) sccGraph[to] = {};

    if (dijkstraGraph.find(to) == dijkstraGraph.end()) dijkstraGraph[to] = {};
    dijkstraGraph[from].push_back({to, distance});
}

// Tarjan Algorithm
vector<vector<string>> TarjanSCC() {
    vector<vector<string>> scc;
    unordered_map<string, int> indices;
    unordered_map<string, int> low;
    unordered_map<string, bool> onStack;
    stack<string> st;
    int currentIndex = 0;

    function<void(const string&)> strongconnect = [&](const string& v) {
        indices[v] = currentIndex;
        low[v] = currentIndex;
        currentIndex++;
        st.push(v);
        onStack[v] = true;

        for (const string& w : sccGraph[v]) {
            if (indices.find(w) == indices.end()) {
                strongconnect(w);
                low[v] = min(low[v], low[w]);
            } else if (onStack[w]) {
                low[v] = min(low[v], indices[w]);
            }
        }

        if (low[v] == indices[v]) {
            vector<string> component;
            string w;
            do {
                w = st.top();
                st.pop();
                onStack[w] = false;
                component.push_back(w);
            } while (w != v);
            scc.push_back(component);
        }
    }; 

    for (const auto& node : sccGraph) {
        if (indices.find(node.first) == indices.end()) {
            strongconnect(node.first);
        }
    }
    return scc;
}

// Path-Based SCC Algorithm
vector<vector<string>> PathBasedSCC() {
    vector<vector<string>> scc;
    unordered_map<string, int> preorder;
    unordered_map<string, int> sccno;
    for (const auto& node : sccGraph) {
        sccno[node.first] = 0;
    }
    stack<string> S, P;
    int index = 0;
    int sccCounter = 0;

    function<void(const string&)> dfs = [&](const string& v) {
        preorder[v] = index++;
        S.push(v);
        P.push(v);

        for (const string& w : sccGraph[v]) {
            if (preorder.find(w) == preorder.end()) {
                dfs(w);
            } else if (sccno[w] == 0) {
                while (!P.empty() && preorder[P.top()] > preorder[w]) {
                    P.pop();
                }
            }
        }

        if (!P.empty() && v == P.top()) {
            P.pop();
            vector<string> component;
            string w;
            do {
                w = S.top();
                S.pop();
                sccno[w] = sccCounter + 1;
                component.push_back(w);
            } while (w != v);
            scc.push_back(component);
            sccCounter++;
        }
    };

    for (const auto& node : sccGraph) {
        if (preorder.find(node.first) == preorder.end()) {
            dfs(node.first);
        }
    }
    return scc;
};

// Dijkstra algorithm
unordered_map<string, double> dijkstra(const string& start) {
    if (dijkstraGraph.find(start) == dijkstraGraph.end()) {
        cerr << "Помилка: стартова вершина '" << start << "' не існує!" << endl;
        return {};
    }

    unordered_map<string, double> distances;
    for (const auto& node_pair : dijkstraGraph) {
        distances[node_pair.first] = numeric_limits<double>::infinity();
    }
    distances[start] = 0.0;

    priority_queue<pair<double, string>, vector<pair<double, string>>, greater<>> pq;
    pq.push({0.0, start});

    while (!pq.empty()) {
        double dist = pq.top().first;
        string current = pq.top().second;
        pq.pop();

        if (dist > distances[current])
            continue;

        for (const auto& edge : dijkstraGraph[current]) {
            string neighbor = edge.first;
            double weight = edge.second;
            if (dist + weight < distances[neighbor]) {
                distances[neighbor] = dist + weight;
                pq.push({distances[neighbor], neighbor});
            }
        }
    }
    return distances;
};

void GraphSetup() {
    AddEdge("Kyiv Polytechnic", "Red University", 4.9);
    AddEdge("Red University", "Kyiv Polytechnic", 4.9);

    AddEdge("Red University", "Golden Gates", 5.7);
    AddEdge("Golden Gates", "Red University", 5.7);

    AddEdge("Golden Gates", "Saint Sophia Cathedral", 5.5);
    AddEdge("Saint Sophia Cathedral", "Golden Gates", 5.5);

    AddEdge("Golden Gates", "Fountain on Khreshchatyk", 5.1);
    AddEdge("Fountain on Khreshchatyk", "St. Michael's Cathedral", 9.5);

    AddEdge("St. Michael's Cathedral", "Saint Sophia Cathedral", 6.4);
    AddEdge("St. Michael's Cathedral", "National Philormony", 5.9);
    AddEdge("National Philormony", "St. Michael's Cathedral", 5.9);

    AddEdge("St. Michael's Cathedral", "Kyiv funicular", 2.5);
    AddEdge("Kyiv funicular", "St. Michael's Cathedral", 2.5);

    AddEdge("Saint Sophia Cathedral", "Lach Gates", 5.8);
    AddEdge("Lach Gates", "National Philormony", 6.2);
    AddEdge("National Philormony", "Lach Gates", 6.2);

    AddEdge("National Philormony", "St. Michael's Cathedral", 5.9);
    AddEdge("Saint Andrew Church", "St. Michael's Cathedral", 4.3);

    AddEdge("Saint Andrew Church", "Saint Sophia Cathedral", 7.1);
    AddEdge("Saint Sophia Cathedral", "Saint Andrew Church", 7.1);
    
    AddEdge("Saint Andrew Church", "Museum of one street", 2.1);
    AddEdge("Museum of one street", "Saint Andrew Church", 2.1);

    AddEdge("Museum of one street", "Kyiv funicular", 3.9);
    AddEdge("Kyiv funicular", "National Philormony", 4.2);
    AddEdge("National Philormony", "Kyiv funicular", 4.2);
}

//Map loading
void LoadImageTexture() {
    unsigned char* data = stbi_load("map.png", &imgWidth, &imgHeight, &imgChannels, 4);
    if (data) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            cerr << "OpenGL error after texture load: " << err << endl;
        }
    } else {
        cerr << "Failed to load map image!" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

// Main initialization
int main() {

    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    int windowWidth = 1280;
    int windowHeight = 620;
    
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Kyiv Path Analyzer", nullptr, nullptr);
    if (!window) {
        cerr << "Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }
    
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    
    if (mode) {
        int xpos = (mode->width - windowWidth) / 2;
        int ypos = (mode->height - windowHeight) / 2;
        glfwSetWindowPos(window, xpos, ypos);
    }
    
    glfwMakeContextCurrent(window);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cerr << "GLEW initialization failed!" << endl;
        glfwTerminate();
        return -1;
    }
    

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImPlotContext* plotCtx = ImPlot::CreateContext();
    if (!plotCtx) { 
        cerr << "Error creating text ImPlot!" << endl;
        glfwTerminate();
        return -1;
    }

    GraphSetup();
    LoadImageTexture();
    dijkstraResult = dijkstra("Kyiv Polytechnic");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Dijkstra Window
        ImGui::Begin("Shortest Distances");
        vector<pair<string, double>> sortedResults(dijkstraResult.begin(), dijkstraResult.end());
        sort(sortedResults.begin(), sortedResults.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        for (const auto& entry : sortedResults) {
            if (entry.first != "Kyiv Polytechnic") {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "→ %s: %.2f km",
                    entry.first.c_str(), entry.second);
            }
        }
        ImGui::End();

        // Tarjan Window
        ImGui::Begin("Tarjan's SCC");
        if (ImGui::Button("Run Tarjan")) {
            auto start = chrono::high_resolution_clock::now();
            tarjanResult = TarjanSCC();
            auto end = chrono::high_resolution_clock::now();
            tarjanTime = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
        }
        ImGui::Text("Time: %.4f ms", tarjanTime);
        for (size_t i = 0; i < tarjanResult.size(); ++i) {
            ImGui::Text("Component %d:", static_cast<int>(i + 1));
            for (const auto& node : tarjanResult[i]) {
                ImGui::BulletText("%s", node.c_str());
            }
        }
        ImGui::End();

        //Path-Based SCC Window
        ImGui::Begin("Path-Based SCC");
        if (ImGui::Button("Run Path-Based")) {
            auto start = chrono::high_resolution_clock::now();
            pathBasedResult = PathBasedSCC();
            auto end = chrono::high_resolution_clock::now();
            pathBasedTime = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
        }
        ImGui::Text("Time: %.4f ms", pathBasedTime);
        for (size_t i = 0; i < pathBasedResult.size(); ++i) {
            ImGui::Text("Component %d:", static_cast<int>(i + 1));
            for (const auto& node : pathBasedResult[i]) {
                ImGui::BulletText("%s", node.c_str());
            }
        }
        ImGui::End();

        // Performance 
        ImGui::SetNextWindowSize(ImVec2(0, 800), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Performance")) {
            const char* labels[] = {"Tarjan", "Path-Based"};
            double times[2] = {tarjanTime * 1000.0, pathBasedTime * 1000.0};
            double maxTime = std::max(times[0], times[1]);

                if (ImPlot::BeginPlot("Time Comparison", ImVec2(-1,300))) {
                ImPlot::SetupAxes("Algorithm", "Time (ms)");
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, maxTime + 0.5, ImGuiCond_Always);
                ImPlot::SetupAxisTicks(ImAxis_X1, 0, 2, 2, labels);
                ImPlot::PlotBars("##Bars", times, 2, 0.5f, 1.0f, 0);
                ImPlot::EndPlot();
                }

        ImGui::End();
        }
        

        //Map
        ImGui::Begin("Map");
        if (texture != 0) {
            ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(imgWidth * 0.8f, imgHeight * 0.8f));
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Map not found!");
            ImGui::Text("Check if map.png exists");
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleaning 
    if (texture != 0) glDeleteTextures(1, &texture);
    if (ImPlot::GetCurrentContext()) {
        ImPlot::DestroyContext();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}