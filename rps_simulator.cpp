//
// Created by Harshwardhan Singh on 17/07/25.
//

#include <iostream>      // For console output
#include <vector>        // For storing game objects
#include <random>        // For random number generation
#include <cmath>         // For mathematical operations (distance, etc.)
#include <chrono>        // For timing and seeding random generator
#include <thread>        // For adding delays in simulation
#include <iomanip>       // For formatted output


enum class ObjectType {
    ROCK,
    PAPER,
    SCISSORS
};

// Convert enum to string for display
std::string typeToString(ObjectType type) {
    switch(type) {
        case ObjectType::ROCK: return "Rock";
        case ObjectType::PAPER: return "Paper";
        case ObjectType::SCISSORS: return "Scissors";
    }
    return "Unknown";
}

// Convert enum to symbol for compact display
char typeToSymbol(ObjectType type) {
    switch(type) {
        case ObjectType::ROCK: return 'R';
        case ObjectType::PAPER: return 'P';
        case ObjectType::SCISSORS: return 'S';
    }
    return '?';
}

class GameObject {
public:
    ObjectType type;
    float x, y;           // Position
    float vx, vy;         // Velocity
    float radius;         // For collision detection

    GameObject(ObjectType t, float startX, float startY)
        : type(t), x(startX), y(startY), radius(5.0f) {
        // Initialize with random velocity
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);

        vx = velDist(gen);
        vy = velDist(gen);
    }

    // Update position based on velocity
    void update() {
        x += vx;
        y += vy;
    }

    // Check collision with another object
    bool collidesWith(const GameObject& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance < (radius + other.radius);
    }

    // Handle boundary bouncing
    void handleBoundaries(float width, float height) {
        if (x - radius <= 0 || x + radius >= width) {
            vx = -vx;
            x = std::max(radius, std::min(width - radius, x));
        }
        if (y - radius <= 0 || y + radius >= height) {
            vy = -vy;
            y = std::max(radius, std::min(height - radius, y));
        }
    }
};

class GameRules {
public:
    // Determine winner between two object types
    static ObjectType determineWinner(ObjectType type1, ObjectType type2) {
        if (type1 == type2) return type1; // No change if same type

        switch(type1) {
            case ObjectType::ROCK:
                return (type2 == ObjectType::SCISSORS) ? ObjectType::ROCK : ObjectType::PAPER;
            case ObjectType::PAPER:
                return (type2 == ObjectType::ROCK) ? ObjectType::PAPER : ObjectType::SCISSORS;
            case ObjectType::SCISSORS:
                return (type2 == ObjectType::PAPER) ? ObjectType::SCISSORS : ObjectType::ROCK;
        }
        return type1;
    }

    // Apply collision result to two objects
    static void resolveCollision(GameObject& obj1, GameObject& obj2) {
        ObjectType winner = determineWinner(obj1.type, obj2.type);

        // Both objects become the winning type
        obj1.type = winner;
        obj2.type = winner;

        // Add some separation to prevent immediate re-collision
        float dx = obj1.x - obj2.x;
        float dy = obj1.y - obj2.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            float separationForce = 2.0f;
            obj1.x += (dx / distance) * separationForce;
            obj1.y += (dy / distance) * separationForce;
            obj2.x -= (dx / distance) * separationForce;
            obj2.y -= (dy / distance) * separationForce;
        }
    }
};

class RPSSimulator {
private:
    std::vector<GameObject> objects;
    float boxWidth, boxHeight;
    std::mt19937 rng;
    int generation;

public:
    RPSSimulator(float width, float height)
        : boxWidth(width), boxHeight(height), generation(0) {
        // Seed random number generator
        rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());

        // Initialize objects
        initializeObjects();
    }

    void initializeObjects() {
        objects.clear();

        std::uniform_real_distribution<float> xDist(10, boxWidth - 10);
        std::uniform_real_distribution<float> yDist(10, boxHeight - 10);

        // Create 5 of each type
        for (int i = 0; i < 5; i++) {
            objects.emplace_back(ObjectType::ROCK, xDist(rng), yDist(rng));
            objects.emplace_back(ObjectType::PAPER, xDist(rng), yDist(rng));
            objects.emplace_back(ObjectType::SCISSORS, xDist(rng), yDist(rng));
        }
    }

    void update() {
        // Update all objects
        for (auto& obj : objects) {
            obj.update();
            obj.handleBoundaries(boxWidth, boxHeight);
        }

        // Check for collisions
        for (size_t i = 0; i < objects.size(); i++) {
            for (size_t j = i + 1; j < objects.size(); j++) {
                if (objects[i].collidesWith(objects[j])) {
                    GameRules::resolveCollision(objects[i], objects[j]);
                }
            }
        }

        generation++;
    }

    // Count objects of each type
    void getTypeCounts(int& rocks, int& papers, int& scissors) const {
        rocks = papers = scissors = 0;

        for (const auto& obj : objects) {
            switch(obj.type) {
                case ObjectType::ROCK: rocks++; break;
                case ObjectType::PAPER: papers++; break;
                case ObjectType::SCISSORS: scissors++; break;
            }
        }
    }

    // Check if game is over (only one type remains)
    bool isGameOver() const {
        int rocks, papers, scissors;
        getTypeCounts(rocks, papers, scissors);

        int nonZeroCount = (rocks > 0) + (papers > 0) + (scissors > 0);
        return nonZeroCount <= 1;
    }

    // Get the winning type
    ObjectType getWinner() const {
        int rocks, papers, scissors;
        getTypeCounts(rocks, papers, scissors);

        if (rocks > 0) return ObjectType::ROCK;
        if (papers > 0) return ObjectType::PAPER;
        if (scissors > 0) return ObjectType::SCISSORS;

        return ObjectType::ROCK; // Fallback
    }

    // Display current state
    void displayState() const {
        int rocks, papers, scissors;
        getTypeCounts(rocks, papers, scissors);

        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "Generation: " << generation << "\n";
        std::cout << "Rocks: " << rocks << " | Papers: " << papers << " | Scissors: " << scissors << "\n";

        // Simple visual representation
        std::cout << "\nSimulation Box (" << boxWidth << "x" << boxHeight << "):\n";
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 40; x++) {
                bool found = false;
                for (const auto& obj : objects) {
                    int objX = static_cast<int>(obj.x * 40 / boxWidth);
                    int objY = static_cast<int>(obj.y * 20 / boxHeight);

                    if (objX == x && objY == y) {
                        std::cout << typeToSymbol(obj.type);
                        found = true;
                        break;
                    }
                }
                if (!found) std::cout << ".";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    std::cout << "Rock Paper Scissors Simulator\n";
    std::cout << "=============================\n\n";

    // Create simulator with a 100x100 box
    RPSSimulator simulator(100.0f, 100.0f);

    std::cout << "Starting simulation with 5 Rocks, 5 Papers, and 5 Scissors...\n";
    std::cout << "Legend: R = Rock, P = Paper, S = Scissors\n";

    // Display initial state
    simulator.displayState();

    // Run simulation
    int maxGenerations = 1000;
    for (int gen = 0; gen < maxGenerations && !simulator.isGameOver(); gen++) {
        simulator.update();

        // Display state every 10 generations
        if (gen % 10 == 0) {
            simulator.displayState();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // Display final result
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "SIMULATION COMPLETE!\n";
    simulator.displayState();

    if (simulator.isGameOver()) {
        std::cout << "\nWinner: " << typeToString(simulator.getWinner()) << "!\n";
    } else {
        std::cout << "\nSimulation ended after maximum generations.\n";
    }

    return 0;
}
