// Update the chase so there a 2 leds on chaseing each other.
// Add a calibration animation
// implement a manual control section taht enables to turn off or on all the leds, and a button which redirects to a page where you can controll all the leds individually, with smart buttons that show on the webpage when on.
// Make variables for the animations cahngeble on the webpage, so fx you can change the speed or the amounts of raindrops in an animation.
// IMprove the webpage to make it intuative and modern
// write code to make the website look good with a modern feel.



#include <WiFi.h>
#include <WebServer.h>
#include "WifiCredentials.h"

class LEDCube {
private:
    int columnPins[16];
    int layerPins[4];
    bool ledState[4][16];

public:
    LEDCube(int cols[16], int layers[4]) {
        for (int i = 0; i < 16; i++) {
            columnPins[i] = cols[i];
        }
        for (int i = 0; i < 4; i++) {
            layerPins[i] = layers[i];
        }
        clear();
    }

    void setupPins() {
        for (int i = 0; i < 16; i++) {
            pinMode(columnPins[i], OUTPUT);
            digitalWrite(columnPins[i], HIGH);
        }
        for (int i = 0; i < 4; i++) {
            pinMode(layerPins[i], OUTPUT);
            digitalWrite(layerPins[i], LOW);
        }
    }

    void setLED(int layer, int col) {
        if (layer < 0 || layer >= 4 || col < 0 || col >= 16) return;
        ledState[layer][col] = true;
    }

    void clear() {
        for (int l = 0; l < 4; l++) {
            for (int c = 0; c < 16; c++) {
                ledState[l][c] = false;
            }
        }
    }

    void display() {
        for (int l = 0; l < 4; l++) {
            digitalWrite(layerPins[l], HIGH);
            for (int c = 0; c < 16; c++) {
                digitalWrite(columnPins[c], ledState[l][c] ? LOW : HIGH);
            }
            delayMicroseconds(200);
            digitalWrite(layerPins[l], LOW);
        }
    }
};

class Animation {
protected:
    LEDCube &cube;

public:
    Animation(LEDCube &c) : cube(c) {}
    virtual void update() = 0;
};

class AnimationController {
private:
    Animation* currentAnimation;
    unsigned long lastUpdate;
    unsigned long animationInterval;

public:
    AnimationController() 
        : currentAnimation(nullptr), lastUpdate(0), animationInterval(100) {}

    bool setAnimation(Animation* animation) {
        if (!animation) {
            Serial.println("Attempted to set null animation");  // Debug output
            return false;
        }
        
        currentAnimation = animation;
        lastUpdate = millis();  // Reset timer
        Serial.println("Animation set successfully");  // Debug output
        return true;
    }

    void update() {
        if (currentAnimation && (millis() - lastUpdate >= animationInterval)) {
            try {
                currentAnimation->update();
                lastUpdate = millis();
            } catch (...) {
                Serial.println("Error in animation update");  // Debug output
                currentAnimation = nullptr;
            }
        }
    }
};

class SparkleAnimation : public Animation {
public:
    SparkleAnimation(LEDCube &c) : Animation(c) {}

    void update() override {
        cube.clear();
        int numLeds = random(5, 11);
        for (int i = 0; i < numLeds; i++) {
            int layer = random(0, 4);
            int col = random(0, 16);
            cube.setLED(layer, col);
        }
    }
};

class RainfallAnimation : public Animation {
private:
    int droplets[5];    // Track multiple raindrops
    int heights[5];     // Track height of each drop
    bool active[5];     // Track if droplet is currently falling

public:
    RainfallAnimation(LEDCube &c) : Animation(c) {
        // Initialize droplets
        for (int i = 0; i < 5; i++) {
            active[i] = false;
            droplets[i] = 0;
            heights[i] = 3;  // Start at top
        }
    }

    void update() override {
        cube.clear();
        
        // Create new droplets randomly
        for (int i = 0; i < 5; i++) {
            if (!active[i] && random(100) < 30) {  // 30% chance to create new droplet
                active[i] = true;
                droplets[i] = random(16);  // Random column
                heights[i] = 3;  // Start at top
            }
        }

        // Update and draw existing droplets
        for (int i = 0; i < 5; i++) {
            if (active[i]) {
                cube.setLED(heights[i], droplets[i]);
                heights[i]--;  // Move down
                
                if (heights[i] < 0) {
                    active[i] = false;  // Deactivate when reaching bottom
                }
            }
        }
    }
};

class RippleAnimation : public Animation {
private:
    int centerX;
    int centerY;
    int radius;
    int maxRadius;
    bool expanding;

public:
    RippleAnimation(LEDCube &c) : Animation(c) {
        resetRipple();
        maxRadius = 4;  // Maximum ripple size
    }

    void resetRipple() {
        centerX = random(2);  // Center point (0-1 for 4x4 grid)
        centerY = random(2);
        radius = 0;
        expanding = true;
    }

    void update() override {
        cube.clear();
        
        // Calculate which LEDs should be lit based on distance from center
        for (int layer = 0; layer < 4; layer++) {
            for (int col = 0; col < 16; col++) {
                // Convert column number to X,Y coordinates (4x4 grid)
                int x = col % 4;
                int y = col / 4;
                
                // Calculate distance from center
                float distance = sqrt(pow(x/2.0 - centerX, 2) + pow(y/2.0 - centerY, 2));
                
                // Light LEDs that are at the current radius
                if (abs(distance - (radius/2.0)) < 0.7) {
                    cube.setLED(layer, col);
                }
            }
        }

        // Update radius
        if (expanding) {
            radius++;
            if (radius >= maxRadius) {
                expanding = false;
            }
        } else {
            radius--;
            if (radius <= 0) {
                resetRipple();
                expanding = true;
            }
        }
    }
};

class PulsingCubeAnimation : public Animation {
private:
    int intensity;
    bool increasing;
    int counter;

public:
    PulsingCubeAnimation(LEDCube &c) : Animation(c) {
        intensity = 0;
        increasing = true;
        counter = 0;
    }

    void update() override {
        cube.clear();
        counter++;
        
        // Update intensity
        if (counter % 2 == 0) {  // Slow down the pulsing
            if (increasing) {
                intensity++;
                if (intensity >= 4) {
                    increasing = false;
                }
            } else {
                intensity--;
                if (intensity <= 0) {
                    increasing = true;
                }
            }
        }

        // Light up LEDs based on current intensity
        for (int layer = 0; layer < intensity; layer++) {
            for (int col = 0; col < 16; col++) {
                cube.setLED(layer, col);
            }
        }
    }
};

class SnakeAnimation : public Animation {
private:
    struct Position {
        int layer;
        int x;
        int y;
        
        bool operator==(const Position& other) const {
            return layer == other.layer && x == other.x && y == other.y;
        }
    };
    
    static const int SNAKE_LENGTH = 4;
    Position snake[SNAKE_LENGTH];  // Fixed-size array for snake segments
    int snakeHead;  // Index of the head in the array
    Position food;
    Position direction;
    int moveCounter;  // To control snake speed
    
public:
    SnakeAnimation(LEDCube &c) : Animation(c) {
        resetSnake();
    }
    
    void resetSnake() {
        // Initialize snake in the middle of the cube
        snakeHead = 0;
        for (int i = 0; i < SNAKE_LENGTH; i++) {
            snake[i] = {1, 1, 1 + i};
        }
        direction = {0, 0, -1};  // Initial direction
        moveCounter = 0;
        placeFood();
    }
    
    void placeFood() {
        bool validPosition;
        do {
            food = {
                random(4),  // layer
                random(4),  // x
                random(4)   // y
            };
            validPosition = true;
            
            // Check if food position conflicts with snake
            for (int i = 0; i < SNAKE_LENGTH; i++) {
                if (snake[i] == food) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);
    }
    
    Position calculateNewHead() {
        Position newHead = snake[snakeHead];
        
        // Update position based on direction
        newHead.layer = (newHead.layer + direction.layer + 4) % 4;
        newHead.x = (newHead.x + direction.x + 4) % 4;
        newHead.y = (newHead.y + direction.y + 4) % 4;
        
        return newHead;
    }
    
    void changeDirection() {
        // Calculate vector to food
        Position head = snake[snakeHead];
        int dl = food.layer - head.layer;
        int dx = food.x - head.x;
        int dy = food.y - head.y;
        
        // Adjust for wrap-around
        if (abs(dl) > 2) dl = (dl > 0) ? dl - 4 : dl + 4;
        if (abs(dx) > 2) dx = (dx > 0) ? dx - 4 : dx + 4;
        if (abs(dy) > 2) dy = (dy > 0) ? dy - 4 : dy + 4;
        
        // Choose direction based on largest difference
        Position newDir = {0, 0, 0};
        if (abs(dl) >= abs(dx) && abs(dl) >= abs(dy)) {
            newDir.layer = (dl > 0) ? 1 : -1;
        } else if (abs(dx) >= abs(dy)) {
            newDir.x = (dx > 0) ? 1 : -1;
        } else {
            newDir.y = (dy > 0) ? 1 : -1;
        }
        
        // Prevent 180-degree turns
        if (!(newDir.layer == -direction.layer && 
              newDir.x == -direction.x && 
              newDir.y == -direction.y)) {
            direction = newDir;
        }
    }
    
    void moveSnake(Position newHead) {
        // Move all segments one position back
        int newHeadIndex = (snakeHead - 1 + SNAKE_LENGTH) % SNAKE_LENGTH;
        snake[newHeadIndex] = newHead;
        snakeHead = newHeadIndex;
    }
    
    void update() override {
        cube.clear();
        
        moveCounter++;
        if (moveCounter >= 5) {  // Control speed of snake
            moveCounter = 0;
            
            // Change direction towards food
            changeDirection();
            
            // Calculate new head position
            Position newHead = calculateNewHead();
            
            // Move snake to new position
            moveSnake(newHead);
            
            // Check if food is eaten
            if (snake[snakeHead] == food) {
                placeFood();
            }
        }
        
        // Draw snake
        for (int i = 0; i < SNAKE_LENGTH; i++) {
            cube.setLED(snake[i].layer, snake[i].x + snake[i].y * 4);
        }
        
        // Draw food
        cube.setLED(food.layer, food.x + food.y * 4);
    }
};

class ExplosionAnimation : public Animation {
private:
    int centerLayer;
    int centerX;
    int centerY;
    int radius;
    bool active;
    
public:
    ExplosionAnimation(LEDCube &c) : Animation(c) {
        resetExplosion();
    }
    
    void resetExplosion() {
        centerLayer = random(4);
        centerX = random(4);
        centerY = random(4);
        radius = 0;
        active = true;
    }
    
    void update() override {
        cube.clear();
        
        if (!active) {
            resetExplosion();
            return;
        }
        
        // Light LEDs based on distance from center
        for (int l = 0; l < 4; l++) {
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    float distance = sqrt(
                        pow(l - centerLayer, 2) +
                        pow(x - centerX, 2) +
                        pow(y - centerY, 2)
                    );
                    
                    // Create shell effect
                    if (abs(distance - radius/2.0) < 1.0) {
                        cube.setLED(l, x + y * 4);
                    }
                }
            }
        }
        
        radius++;
        if (radius > 8) {  // Reset after explosion reaches max size
            active = false;
        }
    }
};

class FireworksAnimation : public Animation {
private:
    struct Particle {
        float layer, x, y;
        float velocityL, velocityX, velocityY;
        int life;
        bool active;
    };
    
    static const int MAX_PARTICLES = 20;
    Particle particles[MAX_PARTICLES];
    int frameCount;
    
public:
    FireworksAnimation(LEDCube &c) : Animation(c) {
        frameCount = 0;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].active = false;
        }
    }
    
    void launchFirework() {
        float startX = random(4);
        float startY = random(4);
        
        // Create burst of particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                particles[i].layer = 0;  // Start at bottom
                particles[i].x = startX;
                particles[i].y = startY;
                
                // Random velocities for explosion effect
                particles[i].velocityL = random(10, 20) / 10.0;
                particles[i].velocityX = (random(-10, 10) / 10.0);
                particles[i].velocityY = (random(-10, 10) / 10.0);
                
                particles[i].life = random(10, 20);
                particles[i].active = true;
            }
        }
    }
    
    void update() override {
        cube.clear();
        frameCount++;
        
        // Launch new firework randomly
        if (random(100) < 5) {  // 5% chance each frame
            launchFirework();
        }
        
        // Update and draw particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                // Update position
                particles[i].layer += particles[i].velocityL * 0.2;
                particles[i].x += particles[i].velocityX * 0.1;
                particles[i].y += particles[i].velocityY * 0.1;
                
                // Apply gravity
                particles[i].velocityL -= 0.1;
                
                // Check if particle is still in cube and alive
                if (particles[i].layer >= 0 && particles[i].layer < 4 &&
                    particles[i].x >= 0 && particles[i].x < 4 &&
                    particles[i].y >= 0 && particles[i].y < 4 &&
                    particles[i].life > 0) {
                    
                    // Draw particle
                    cube.setLED(
                        int(particles[i].layer),
                        int(particles[i].x) + int(particles[i].y) * 4
                    );
                    
                    particles[i].life--;
                } else {
                    particles[i].active = false;
                }
            }
        }
    }
};

class TwisterAnimation : public Animation {
private:
    float angle;
    float height;
    bool growing;
    float radius;
    
public:
    TwisterAnimation(LEDCube &c) : Animation(c) {
        angle = 0;
        height = 0;
        growing = true;
        radius = 0.5;
    }
    
    void update() override {
        cube.clear();
        
        // Update spiral parameters
        angle += 0.3;  // Rotation speed
        if (growing) {
            radius += 0.1;
            if (radius >= 1.5) growing = false;
        } else {
            radius -= 0.1;
            if (radius <= 0.5) growing = true;
        }
        
        // Draw spiral pattern
        for (float h = 0; h < 4; h += 0.5) {
            float currentAngle = angle + (h * 2);  // Twist more as we go up
            float x = 2 + cos(currentAngle) * radius;
            float y = 2 + sin(currentAngle) * radius;
            
            if (x >= 0 && x < 4 && y >= 0 && y < 4) {
                cube.setLED(int(h), int(x) + int(y) * 4);
            }
        }
    }
};

class CubeRotationAnimation : public Animation {
private:
    float angleX, angleY, angleZ;
    struct Point3D {
        float x, y, z;
    };
    std::vector<Point3D> cubePoints;
    
public:
    CubeRotationAnimation(LEDCube &c) : Animation(c) {
        angleX = angleY = angleZ = 0;
        
        // Define cube corners
        for (int x = 0; x < 2; x++) {
            for (int y = 0; y < 2; y++) {
                for (int z = 0; z < 2; z++) {
                    cubePoints.push_back({float(x)*3-1.5f, float(y)*3-1.5f, float(z)*3-1.5f});
                }
            }
        }
    }
    
    Point3D rotatePoint(Point3D p) {
        // Rotate around X axis
        float y = p.y * cos(angleX) - p.z * sin(angleX);
        float z = p.y * sin(angleX) + p.z * cos(angleX);
        p.y = y;
        p.z = z;
        
        // Rotate around Y axis
        float x = p.x * cos(angleY) + z * sin(angleY);
        z = -p.x * sin(angleY) + z * cos(angleY);
        p.x = x;
        
        // Rotate around Z axis
        y = p.y * cos(angleZ) - x * sin(angleZ);
        x = p.y * sin(angleZ) + x * cos(angleZ);
        
        return {x, y, z};
    }
    
    void update() override {
        cube.clear();
        
        // Update rotation angles
        angleX += 0.05;
        angleY += 0.03;
        angleZ += 0.04;
        
        // Rotate and project all points
        for (const auto& point : cubePoints) {
            Point3D rotated = rotatePoint(point);
            
            // Project to cube space (0-3)
            int x = int((rotated.x + 2) * 0.8);
            int y = int((rotated.y + 2) * 0.8);
            int z = int((rotated.z + 2) * 0.8);
            
            // Draw if within bounds
            if (x >= 0 && x < 4 && y >= 0 && y < 4 && z >= 0 && z < 4) {
                cube.setLED(z, x + y * 4);
            }
        }
    }
};

class LightChaseAnimation : public Animation {
private:
    enum class PathType { HORIZONTAL, VERTICAL, DIAGONAL };
    struct Point {
        int layer, col;
        bool isPredator;  // true for chaser, false for being chased
    };
    std::vector<Point> path;
    int predatorIndex;
    int preyIndex;
    int pathType;
    int displayCounter;  // Counter for simulating brightness
    
public:
    LightChaseAnimation(LEDCube &c) : Animation(c) {
        generatePath(PathType::HORIZONTAL);
        predatorIndex = 0;
        preyIndex = path.size() / 2;  // Start prey halfway through the path
        pathType = 0;
        displayCounter = 0;
    }
    
    void generatePath(PathType type) {
        path.clear();
        
        switch(type) {
            case PathType::HORIZONTAL:
                // Generate snake-like horizontal path
                for (int layer = 0; layer < 4; layer++) {
                    if (layer % 2 == 0) {
                        for (int col = 0; col < 16; col++) {
                            path.push_back({layer, col, false});
                        }
                    } else {
                        for (int col = 15; col >= 0; col--) {
                            path.push_back({layer, col, false});
                        }
                    }
                }
                break;
                
            case PathType::VERTICAL:
                // Generate vertical zigzag path
                for (int x = 0; x < 4; x++) {
                    for (int y = 0; y < 4; y++) {
                        for (int layer = 0; layer < 4; layer++) {
                            path.push_back({layer, x + y * 4, false});
                        }
                    }
                }
                break;
                
            case PathType::DIAGONAL:
                // Generate diagonal path
                for (int d = 0; d < 4; d++) {
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            if (i + j == d) {
                                path.push_back({i, j * 4 + d, false});
                            }
                        }
                    }
                }
                break;
        }
    }
    
    void update() override {
        cube.clear();
        displayCounter = (displayCounter + 1) % 4;  // Used for brightness control
        
        // Always show predator (bright LED)
        cube.setLED(path[predatorIndex].layer, path[predatorIndex].col);
        
        // Show prey (dim LED) only on certain display cycles to simulate lower brightness
        if (displayCounter < 2) {  // Show prey LED 50% of the time
            cube.setLED(path[preyIndex].layer, path[preyIndex].col);
        }
        
        // Update positions every few cycles to control speed
        if (displayCounter == 0) {
            // Move predator
            predatorIndex = (predatorIndex + 1) % path.size();
            
            // Move prey
            if (random(100) < 70) {  // 70% chance to move normally
                preyIndex = (preyIndex + 1) % path.size();
            } else {  // 30% chance to move faster (trying to escape)
                preyIndex = (preyIndex + 2) % path.size();
            }
            
            // Check if predator caught prey
            if (predatorIndex == preyIndex) {
                // Reset positions when caught
                predatorIndex = 0;
                preyIndex = path.size() / 2;
                
                // Change pattern
                pathType = (pathType + 1) % 3;
                generatePath(static_cast<PathType>(pathType));
            }
        }
    }
};

class CalibrationAnimation : public Animation {
private:
    enum class TestPhase {
        INDIVIDUAL_LEDS,     // Test each LED one by one
        LAYER_SWEEP,         // Test each layer
        COLUMN_SWEEP,        // Test each column
        BRIGHTNESS_TEST,     // Test brightness fading
        WAVE_TEST,          // Test wave pattern
        FULL_CUBE_TEST,     // Test all LEDs
        COMPLETE            // Calibration complete
    };
    
    TestPhase currentPhase;
    int currentStep;
    int delayCounter;
    float brightness;
    bool fadeDirection;  // true = getting brighter, false = getting dimmer
    
public:
    CalibrationAnimation(LEDCube &c) : Animation(c) {
        resetCalibration();
    }
    
    void resetCalibration() {
        currentPhase = TestPhase::INDIVIDUAL_LEDS;
        currentStep = 0;
        delayCounter = 0;
        brightness = 0;
        fadeDirection = true;
    }
    
    void update() override {
        cube.clear();
        delayCounter++;
        
        switch(currentPhase) {
            case TestPhase::INDIVIDUAL_LEDS:
                updateIndividualTest();
                break;
                
            case TestPhase::LAYER_SWEEP:
                updateLayerTest();
                break;
                
            case TestPhase::COLUMN_SWEEP:
                updateColumnTest();
                break;
                
            case TestPhase::BRIGHTNESS_TEST:
                updateBrightnessTest();
                break;
                
            case TestPhase::WAVE_TEST:
                updateWaveTest();
                break;
                
            case TestPhase::FULL_CUBE_TEST:
                updateFullCubeTest();
                break;
                
            case TestPhase::COMPLETE:
                updateComplete();
                break;
        }
    }
    
private:
    void updateIndividualTest() {
        // Test each LED individually
        if (delayCounter >= 1) {  // Delay between LEDs
            delayCounter = 0;
            currentStep++;
            
            if (currentStep >= 64) {  // 4x4x4 = 64 LEDs
                currentPhase = TestPhase::LAYER_SWEEP;
                currentStep = 0;
                return;
            }
        }
        
        // Calculate current LED position
        int layer = currentStep / 16;
        int col = currentStep % 16;
        cube.setLED(layer, col);
    }
    
    void updateLayerTest() {
        if (delayCounter >= 8) {
            delayCounter = 0;
            currentStep++;
            
            if (currentStep >= 12) {  // 3 complete sweeps
                currentPhase = TestPhase::COLUMN_SWEEP;
                currentStep = 0;
                return;
            }
        }
        
        // Light up current layer
        int layer = (currentStep % 4);
        for (int col = 0; col < 16; col++) {
            cube.setLED(layer, col);
        }
    }
    
    void updateColumnTest() {
        if (delayCounter >= 8) {
            delayCounter = 0;
            currentStep++;
            
            if (currentStep >= 48) {  // 3 complete column sweeps
                currentPhase = TestPhase::BRIGHTNESS_TEST;
                currentStep = 0;
                return;
            }
        }
        
        // Light up current column
        int col = (currentStep % 16);
        for (int layer = 0; layer < 4; layer++) {
            cube.setLED(layer, col);
        }
    }
    
    void updateBrightnessTest() {
        if (delayCounter >= 2) {
            delayCounter = 0;
            currentStep++;
            
            if (currentStep >= 200) {  // Complete fade cycle
                currentPhase = TestPhase::WAVE_TEST;
                currentStep = 0;
                return;
            }
            
            // Update brightness
            if (fadeDirection) {
                brightness += 0.05;
                if (brightness >= 1.0) {
                    fadeDirection = false;
                }
            } else {
                brightness -= 0.05;
                if (brightness <= 0.0) {
                    fadeDirection = true;
                }
            }
        }
        
        // Show all LEDs with current brightness
        if (currentStep % int(1.0 / brightness + 1) == 0) {
            for (int layer = 0; layer < 4; layer++) {
                for (int col = 0; col < 16; col++) {
                    cube.setLED(layer, col);
                }
            }
        }
    }
    
    void updateWaveTest() {
        if (delayCounter >= 3) {
            delayCounter = 0;
            currentStep++;
            
            if (currentStep >= 100) {  // Complete wave cycle
                currentPhase = TestPhase::FULL_CUBE_TEST;
                currentStep = 0;
                return;
            }
        }
        
        // Create wave pattern
        float angle = currentStep * 0.2;
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                int height = int(1.5 + sin(angle + sqrt(x*x + y*y) * 0.5) * 1.5);
                if (height >= 0 && height < 4) {
                    cube.setLED(height, x + y * 4);
                }
            }
        }
    }
    
    void updateFullCubeTest() {
        if (delayCounter >= 10) {
            delayCounter = 0;
            currentStep++;
            
            if (currentStep >= 6) {  // 3 on/off cycles
                currentPhase = TestPhase::COMPLETE;
                currentStep = 0;
                return;
            }
        }
        
        // Toggle all LEDs on/off
        if (currentStep % 2 == 0) {
            for (int layer = 0; layer < 4; layer++) {
                for (int col = 0; col < 16; col++) {
                    cube.setLED(layer, col);
                }
            }
        }
    }
    
    void updateComplete() {
        // Show completion pattern - spiral from bottom to top
        float t = millis() * 0.001;  // Time in seconds
        for (int layer = 0; layer < 4; layer++) {
            float angle = t * 3 + layer * 0.5;
            int x = int(2 + cos(angle) * 1.5);
            int y = int(2 + sin(angle) * 1.5);
            if (x >= 0 && x < 4 && y >= 0 && y < 4) {
                cube.setLED(layer, x + y * 4);
            }
        }
        
        // Reset calibration after a while
        if (currentStep++ > 200) {
            resetCalibration();
        }
    }
};

class AnimationRegistry {
private:
    struct AnimationEntry {
        String name;
        Animation* animation;
        String description;
    };
    
    std::vector<AnimationEntry> animations;
    LEDCube& cube;

public:
    AnimationRegistry(LEDCube& c) : cube(c) {}

    void registerAnimation(const String& name, Animation* animation, const String& description = "") {
        animations.push_back({name, animation, description});
    }

    std::vector<String> getAnimationNames() {
        std::vector<String> names;
        for (const auto& entry : animations) {
            names.push_back(entry.name);
        }
        return names;
    }

    Animation* getAnimation(const String& name) {
        for (const auto& entry : animations) {
            if (entry.name == name) {
                return entry.animation;
            }
        }
        return nullptr;
    }

    String getAnimationListHTML() {
        String html;
        for (const auto& entry : animations) {
            html += "<button class='button' onclick='setAnimation(\"" + entry.name + "\")' "
                   "data-animation='" + entry.name + "'>" + entry.name;
            if (entry.description.length() > 0) {
                html += "<br><small>" + entry.description + "</small>";
            }
            html += "</button>";
        }
        return html;
    }

    void setup() {
        // Register all animations here
        registerAnimation("sparkle", new SparkleAnimation(cube), "Random sparkling effects");
        registerAnimation("rainfall", new RainfallAnimation(cube), "Simulated falling rain drops");
        registerAnimation("ripple", new RippleAnimation(cube), "Expanding circular waves");
        registerAnimation("pulsing", new PulsingCubeAnimation(cube), "Breathing cube effect");
        registerAnimation("snake", new SnakeAnimation(cube), "Snake slithering through the cube");
        registerAnimation("explosion", new ExplosionAnimation(cube), "Expanding explosion effect");
        registerAnimation("fireworks", new FireworksAnimation(cube), "Bursting fireworks display");
        registerAnimation("twister", new TwisterAnimation(cube), "Spinning spiral pattern");
        registerAnimation("rotation", new CubeRotationAnimation(cube), "3D cube rotation effect");
        registerAnimation("chase", new LightChaseAnimation(cube), "Fast-moving light patterns");
        registerAnimation("calibration", new CalibrationAnimation(cube), "Test all LEDs and cube functionality");
    }
};

// Simplified Web Server Handler
class WebInterface {
private:
    WebServer& server;
    AnimationRegistry& registry;
    AnimationController& controller;
    String currentAnimation;

public:
    WebInterface(WebServer& s, AnimationRegistry& r, AnimationController& c)
        : server(s), registry(r), controller(c), currentAnimation("sparkle") {}

    void setup() {
        // Root page
        server.on("/", HTTP_GET, [this]() {
            sendMainPage();
        });

        // Handle animation changes via AJAX
        server.on("/setAnimation", HTTP_GET, [this]() {
            String name = server.arg("name");
            if (name.length() > 0) {
                activateAnimation(name);
            } else {
                server.send(400, "text/plain", "Animation name required");
            }
        });

        // Handle 404
        server.onNotFound([this]() {
            server.send(404, "text/plain", "Page not found");
        });
    }

private:
    void sendMainPage() {
        String html = F("<!DOCTYPE html>"
            "<html><head>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
            "<title>LED Cube Controller</title>"
            "<style>"
                "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }"
                ".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; "
                "text-align: center; text-decoration: none; display: inline-block; font-size: 16px; "
                "margin: 4px 2px; cursor: pointer; border-radius: 4px; width: 200px; }"
                ".button:hover { background-color: #45a049; }"
                ".button.active { background-color: #357abd; }"
                "#status { margin: 20px 0; padding: 10px; border-radius: 4px; background-color: #fff; }"
                ".animation-buttons { display: grid; grid-template-columns: repeat(auto-fill, minmax(220px, 1fr)); gap: 10px; }"
                "#brightnessControl { margin: 20px 0; }"
                "input[type='range'] { width: 100%; max-width: 300px; }"
            "</style>"
            "<script>"
                "function setAnimation(name) {"
                    "fetch('/setAnimation?name=' + encodeURIComponent(name))"
                    ".then(response => response.text())"
                    ".then(text => {"
                        "document.getElementById('status').textContent = text;"
                        "document.querySelectorAll('.button').forEach(btn => {"
                            "btn.classList.remove('active');"
                            "if(btn.getAttribute('data-animation') === name) {"
                                "btn.classList.add('active');"
                            "}"
                        "});"
                    "})"
                    ".catch(error => {"
                        "document.getElementById('status').textContent = 'Error: ' + error;"
                    "});"
                    "return false;"  // Prevent default button behavior
                "}"
            "</script>"
            "</head><body>"
            "<h1>LED Cube Controller</h1>"
            "<div id='status'>Current animation: ");
        
        html += currentAnimation;
        html += F("</div>"
            "<div class='animation-buttons'>");
        html += registry.getAnimationListHTML();
        html += F("</div></body></html>");
        
        server.send(200, "text/html", html);
    }

    void activateAnimation(const String& name) {
        Animation* anim = registry.getAnimation(name);
        if (anim && controller.setAnimation(anim)) {
            currentAnimation = name;
            server.send(200, "text/plain", "Activated " + name + " animation");
        } else {
            server.send(500, "text/plain", "Failed to set animation");
        }
    }
};

// Global objects
int columns[16] = {32, 33, 25, 26, 27, 14, 12, 13, 15, 2, 0, 4, 16, 17, 5, 18};
int layers[4] = {19, 21, 22, 23};
LEDCube cube(columns, layers);
AnimationController animController;
AnimationRegistry animRegistry(cube);
WebServer server(80);
WebInterface webInterface(server, animRegistry, animController);

void setup() {
    Serial.begin(115200);  
    Serial.println("\nStarting LED Cube...");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    cube.setupPins();
    animRegistry.setup();
    webInterface.setup();
    server.begin();

    // Set default animation
    Animation* defaultAnim = animRegistry.getAnimation("sparkle");
    if (defaultAnim) {
        animController.setAnimation(defaultAnim);
    }
}

void loop() {
    server.handleClient();
    animController.update();
    cube.display();
}

