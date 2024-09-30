#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"

struct Circle {
    Vector2 position;
    Vector2 velocity;
    Color circleColor;
    float radius;
};

bool sphereCollisionDetection(Circle& c1, Circle& c2);
void Gravity(Circle circles[], int circleCount, float deltaTime, float gravity, float velocityDamping);
void collisionPhysics(Circle& c1, Circle& c2, float restitution, float damping);

int main() {
    InitWindow(500, 600, "SandSim");
    SetTargetFPS(120);

    int numberOfCircles = 2;
    float deltaTime = 0;
    float gravity = 0.0f;
    float velocityDamping = 0.8f;
    float restitution = 0.1f;
    int radius = 10;

    Circle* circles = (Circle*)malloc(numberOfCircles * sizeof(Circle));

    for (int i = 0; i < numberOfCircles; i++) {
        circles[i].position = (Vector2){ GetRandomValue(100, 700), GetRandomValue(100, 500) };
        circles[i].velocity = (Vector2){ 0.0f, 0.0f };  // Initialize velocity to zero
        circles[i].radius = radius;
        circles[i].circleColor = BLUE;  // Set initial color to BLUE
    }

    rlImGuiSetup(true);

    while (!WindowShouldClose()) {
        // Movement
        Gravity(circles, numberOfCircles, GetFrameTime(), gravity, velocityDamping);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Collision detection and response
        for (int i = 0; i < numberOfCircles; i++) {
            for (int j = i + 1; j < numberOfCircles; j++) {
                if (sphereCollisionDetection(circles[i], circles[j])) {
                    collisionPhysics(circles[i], circles[j], restitution, velocityDamping);  // Using restitution coefficient
                }
            }
        }

        // Reset all circles to BLUE before detecting collisions
        for (int i = 0; i < numberOfCircles; i++) {
            circles[i].circleColor = BLUE;
        }

        // Draw circles
        for (int i = 0; i < numberOfCircles; i++) {
            DrawCircleV(circles[i].position, circles[i].radius, circles[i].circleColor);
        }

        static int prevCircleNum = numberOfCircles;

        // ImGui window
        rlImGuiBegin();
            ImGui::Text("CONTROLS:");
            ImGui::InputInt("Number Of Circles", &numberOfCircles, 1);
            ImGui::SliderFloat("Gravity", &gravity, 0.0f, 100.0f);
            ImGui::SliderFloat("Velocity Damping Factor", &velocityDamping, 0.0f, 1.0f);
            ImGui::SliderFloat("Restitution", &restitution, 0.0f, 0.1f);
            ImGui::InputInt("Radius of particles", &radius, 1);

            // Reallocating the circle array
            if (numberOfCircles != prevCircleNum) {
                if (numberOfCircles > 0) {
                    // Reallocate memory for the new number of circles
                    circles = (Circle*)realloc(circles, numberOfCircles * sizeof(Circle));

                    // Initialize new circles if the number increased
                    if (numberOfCircles > prevCircleNum) {
                        for (int i = prevCircleNum; i < numberOfCircles; i++) {
                            circles[i].position = (Vector2){ GetRandomValue(100, 400), GetRandomValue(100, 500) };
                            circles[i].velocity = (Vector2){ 0.0f, 0.0f };  // Set initial velocity to zero
                            circles[i].radius = radius;
                            circles[i].circleColor = BLUE;
                        }
                    }
                    prevCircleNum = numberOfCircles;
                } else {
                    numberOfCircles = prevCircleNum;
                }
            }

        rlImGuiEnd();

        EndDrawing();
    }

    free(circles);
}

bool sphereCollisionDetection(Circle& c1, Circle& c2) {
    float distance = Vector2Distance(c1.position, c2.position);
    return (distance <= (c1.radius + c2.radius));  // Return true if there is a collision
}

void Gravity(Circle circles[], int circleCount, float deltaTime, float gravity, float velocityDamping) {
    for (int i = 0; i < circleCount; i++) {
        // Apply gravity to the velocity
        circles[i].velocity.y += gravity * deltaTime;

        // Update position based on velocity
        circles[i].position = Vector2Add(circles[i].position, Vector2Scale(circles[i].velocity, deltaTime));

        // Check for collision with the ground (bottom edge)
        if (circles[i].position.y + circles[i].radius > GetScreenHeight()) {
            circles[i].position.y = GetScreenHeight() - circles[i].radius;
            circles[i].velocity.y *= -velocityDamping;  // Bounce back
            if (fabs(circles[i].velocity.y) < 1.0f) {
                circles[i].velocity.y = 0.0f;  // Stop small movements
            }
        }

        // Check for collision with the top edge
        if (circles[i].position.y - circles[i].radius < 0) {
            circles[i].position.y = circles[i].radius;
            circles[i].velocity.y *= -velocityDamping;  // Bounce back
            if (fabs(circles[i].velocity.y) < 1.0f) {
                circles[i].velocity.y = 0.0f;  // Stop small movements
            }
        }

        // Check for collision with the left edge
        if (circles[i].position.x - circles[i].radius < 0) {
            circles[i].position.x = circles[i].radius;
            circles[i].velocity.x *= -velocityDamping;  // Bounce back
            if (fabs(circles[i].velocity.x) < 1.0f) {
                circles[i].velocity.x = 0.0f;  // Stop small movements
            }
        }

        // Check for collision with the right edge
        if (circles[i].position.x + circles[i].radius > GetScreenWidth()) {
            circles[i].position.x = GetScreenWidth() - circles[i].radius;
            circles[i].velocity.x *= -velocityDamping;  // Bounce back
            if (fabs(circles[i].velocity.x) < 1.0f) {
                circles[i].velocity.x = 0.0f;  // Stop small movements
            }
        }
    }
}

void collisionPhysics(Circle& c1, Circle& c2, float restitution, float damping) {
    // Normal direction for collision
    Vector2 collisionNormal = Vector2Subtract(c1.position, c2.position);
    float distance = Vector2Length(collisionNormal);
    if (distance == 0.0f) { return; }  // Prevent division by zero
    collisionNormal = Vector2Scale(collisionNormal, 1.0f / distance);  // Normalize

    Vector2 velocityRelative = Vector2Subtract(c1.velocity, c2.velocity);
    float velocityAlongNormal = Vector2DotProduct(velocityRelative, collisionNormal);
    if (velocityAlongNormal > 0) { return; }  // Prevents collision response if moving apart

    // Impulse calculation
    float impulseStrength = -(1 + restitution) * velocityAlongNormal;
    impulseStrength /= (1 / c1.radius + 1 / c2.radius);  // Assume mass to be 1 unit for simplicity

    // Apply the impulse to the circles' velocities
    Vector2 impulse = Vector2Scale(collisionNormal, impulseStrength * damping);
    c1.velocity = Vector2Add(c1.velocity, impulse);
    c2.velocity = Vector2Subtract(c2.velocity, impulse);  // Note the subtraction for the second circle
}
