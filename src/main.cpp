#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include "../include/sfmlbutton.hpp"

class Disk {
public:
    Disk(float width, float height, sf::Color color, int num) {
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(color);
        this->num = num;
    }

    void setPosition(float x, float y) {
        shape.setPosition(x, y);
    }

    sf::Vector2f getPosition() {
        return shape.getPosition();
    }

    int getNum() {
        return num;
    }

    sf::Color getColor() {
        return shape.getFillColor();
    }

    sf::RectangleShape& getShape() {
        return shape;
    }

private:
    sf::RectangleShape shape;
    int num;
};

class Tower {
public:
    Tower(float x, float y, char letter) : x(x), y(y), letter(letter) {}
    Tower(char letter) : x(0), y(0), letter(letter) {}

    void addDisk(Disk disk) {
        float diskHeight = disk.getShape().getSize().y;
        disk.setPosition(x - disk.getShape().getSize().x / 2, y - (disks.size() + 1) * diskHeight);
        disks.push(disk);
    }

    Disk removeDisk() {
        Disk topDisk = disks.top();
        disks.pop();
        return topDisk;
    }

    void reset() {
        while (!disks.empty()) {
            disks.pop();
        }
    }

    bool isEmpty() const {
        return disks.empty();
    }

    char getLetter() const {
        return letter;
    }

    Disk& getTopDisk() {
        return disks.top();
    }

    sf::Vector2f getPosition() const {
        return sf::Vector2f(x, y);
    }

    void setPosition(float x, float y) {
        this->x = x;
        this->y = y;
    }

    std::stack<Disk>& getDisks() {
        return disks;
    }

private:
    float x, y;
    char letter;
    std::stack<Disk> disks;
};

class Operation {
    public:
        char sourceLetter;
        char destinationLetter;
        int diskNum;
        Operation(char sourceLetter, char destinationLetter, int diskNum) 
            : sourceLetter(sourceLetter), destinationLetter(destinationLetter), diskNum(diskNum) {}
};

void moveDisk(Tower& source, Tower& destination, std::vector<Operation>& operations, bool log = true) {
    Disk disk = source.removeDisk();
    destination.addDisk(disk);
    // Draw towers and disks
    if (log) {
        operations.push_back(Operation(source.getLetter(), destination.getLetter(), disk.getNum()));
    }
    std::vector<Tower*> towers = { &source, &destination };
    for (Tower* tower : towers) {
        std::stack<Disk> disks = tower->getDisks();
        while (!disks.empty()) {
            disks.pop();
        }
    }
}

void solveHanoi(int n, Tower& source, Tower& auxiliary, Tower& destination, std::vector<Operation>& operations, bool log = true) {
    if (n == 1) {
        moveDisk(source, destination, operations, log);
        return;
    }
    solveHanoi(n - 1, source, destination, auxiliary, operations, log);
    moveDisk(source, destination, operations, log);
    solveHanoi(n - 1, auxiliary, source, destination, operations, log);
}

void setDisks(Tower &a, Tower &b, Tower &c, int numDisks, const float windowWidth, const float diskHeight, std::vector<sf::Color> &colors) {
    a.reset();
    b.reset();
    c.reset();
    const float minWidth = 10.f;
    const float factor = ((windowWidth / 4 - 20) - minWidth) / numDisks;
    for (int i = 0; i < numDisks; ++i) {
        float diskWidth = windowWidth / 4 - i * factor;
        a.addDisk(Disk(diskWidth, diskHeight, colors[i % colors.size()], i));
    }
}

sf::Color inverseLegibleColor(sf::Color color) {
    int r = color.r;
    int g = color.g;
    int b = color.b;
    if (r * 0.299 + g * 0.7 + b * 0.114 > 150) {
        return sf::Color::Black;
    } else {
        return sf::Color::White;
    }
}

template<typename T>
T clamp(T value, T min, T max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

float linearInterpolation(float a, float b, float t) {
    return a + (b - a) * t;
}

float getTowerHeight(int numDisks) {
    float minHeight = 100.f;
    float maxHeight = 480.f;
    float factor = (maxHeight - minHeight) / 15;
    return clamp(minHeight + numDisks * factor, minHeight, maxHeight);
}

void animateDiskMove(Disk &disk, const sf::Vector2f init, const sf::Vector2f goal, const float towerMax, const float delta) {
    // the function will only use the delta to determine the position of the disk
    // the delta is te range from 0-1 that describes the completion of the animation
    float x, y;
    if (delta < (3/8.f)) {
        x = init.x;
        y = linearInterpolation(init.y, towerMax, delta / (3/8.f));
    } else if (delta < (5/8.f)) {
        x = linearInterpolation(init.x, goal.x, (delta - 3/8.f) / (5/8.f - 3/8.f));
        y = towerMax;
    } else {
        x = goal.x;
        y = linearInterpolation(towerMax, goal.y, (delta - 5/8.f) / (1 - 5/8.f));
    }
    
    disk.setPosition(x, y);
};

void restart(bool &iniciadoVisualizacion, int &indiceOperacion, Tower &a, Tower &b, Tower &c, int numDisks, const float windowWidth, const float diskHeight, std::vector<sf::Color> &colors, std::vector<Operation> &operations, RectButton &buttonPlus, RectButton &buttonMinus, RectButton &startButton, RectButton &restartButton) {
    std::cout << "Reiniciando..." << std::endl;
    iniciadoVisualizacion = false;
    indiceOperacion = 0;
    setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);
    operations.clear();
    buttonPlus.setButtonEnabled(true);
    buttonMinus.setButtonEnabled(true);
    restartButton.setButtonEnabled(false);
    startButton.setButtonEnabled(true);
};

void calculateTowersPos(Tower &a, const float windowWidth, const float windowHeight, float towerHeight, Tower &b, Tower &c, sf::RectangleShape &base, sf::Text &labelA, sf::Text &labelB, sf::Text &labelC);

int calcularNMovimientos(int numDiscos) {
    return pow(2, numDiscos) - 1;
}

int main() {
    const unsigned int FPS = 60;
    int numDisks = 3;
    int numMoves = calcularNMovimientos(numDisks);
    const float windowWidth = 900;
    const float windowHeight = 600;
    float towerHeight = getTowerHeight(numDisks);
    const float towerWidth = 20;
    const float diskHeight = 30;
    std::vector<Operation> operations;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight + 50), "Torre de Hanoi");
    window.setFramerateLimit(FPS);

    Tower a('A');
    Tower b('B');
    Tower c('C');

    std::vector<sf::Color> colors = { sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta };

    std::vector<Tower*> towers = { &a, &b, &c };

    sf::Font buttonFont;
    buttonFont.loadFromFile("./fonts/Arial.ttf");

    // Gráficos
    sf::RectangleShape base;
    base.setSize(sf::Vector2f(windowWidth - 100, towerWidth + 6));
    base.setFillColor(sf::Color::White);
    sf::Text labelA("A", buttonFont, 14);
    labelA.setFillColor(sf::Color::Black);
    sf::Text labelB("B", buttonFont, 14);
    labelB.setFillColor(sf::Color::Black);
    sf::Text labelC("C", buttonFont, 14);
    labelC.setFillColor(sf::Color::Black);

    calculateTowersPos(a, windowWidth, windowHeight, towerHeight, b, c, base, labelA, labelB, labelC);
    setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);

    // Controles inicio
    sf::Text ndisksText("n: " + std::to_string(numDisks), buttonFont, 20);
    ndisksText.setPosition(50, windowHeight - 30);
    ndisksText.setFillColor(sf::Color::White);
    RectButton buttonPlus(buttonFont, sf::Vector2f(40.f, 20.f), sf::Vector2f(100.f, windowHeight));
    buttonPlus.setButtonLabel(20.f, " + ");
    RectButton buttonMinus(buttonFont, sf::Vector2f(40.f, 20.f), sf::Vector2f(50.f, windowHeight));
    buttonMinus.setButtonLabel(20.f, " - ");
    RectButton startButton(buttonFont, sf::Vector2f(190.f, 30.f), sf::Vector2f(windowWidth - 190 - 50, windowHeight));
    startButton.setButtonLabel(20.f, "Iniciar visualizacion");
    startButton.setButtonColor(sf::Color(0, 200, 0), sf::Color(0, 150, 0), sf::Color(0, 100, 0));
    startButton.setLabelColor(sf::Color::White);

    // Controles visualizacion
    sf::Text currentOperationText("Operacion: --", buttonFont, 20);
    currentOperationText.setPosition(50, windowHeight - 30);
    currentOperationText.setFillColor(sf::Color::White);
    RectButton restartButton(buttonFont, sf::Vector2f(190.f, 30.f), sf::Vector2f(windowWidth - 190 - 50, windowHeight));
    restartButton.setButtonLabel(20.f, "Reiniciar");
    restartButton.setButtonColor(sf::Color(0, 200, 0), sf::Color(0, 150, 0), sf::Color(0, 100, 0));
    restartButton.setLabelColor(sf::Color::White);
    restartButton.setButtonEnabled(false);

    // Estado
    bool iniciadoVisualizacion = false;
    int indiceOperacion = 0;
    bool animating = false;
    float delta = 0.f;
    sf::Vector2f goal;
    sf::Vector2f init;
    Tower* currentSource = nullptr;
    Tower* currentDestination = nullptr;
    Disk* currentDisk = nullptr;

    sf::Event ev;
    while (window.isOpen()) {
        // Events
        while (window.pollEvent(ev)) {
            buttonPlus.getButtonStatus(window, ev);
            buttonMinus.getButtonStatus(window, ev);
            startButton.getButtonStatus(window, ev);
            restartButton.getButtonStatus(window, ev);
            if (ev.type == sf::Event::Closed) {
                window.close();
            }

            if (buttonPlus.isPressed) {
                if (numDisks < 15) {
                    numDisks++;
                    numMoves = calcularNMovimientos(numDisks);
                    towerHeight = getTowerHeight(numDisks);
                    calculateTowersPos(a, windowWidth, windowHeight, towerHeight, b, c, base, labelA, labelB, labelC);
                    setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);
                }
            }

            if (buttonMinus.isPressed) {
                if (numDisks > 1) {
                    numDisks--;
                    numMoves = calcularNMovimientos(numDisks);
                    towerHeight = getTowerHeight(numDisks);
                    calculateTowersPos(a, windowWidth, windowHeight, towerHeight, b, c, base, labelA, labelB, labelC);
                    setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);
                }
            }

            if (startButton.isPressed) {
                solveHanoi(numDisks, a, b, c, operations);
                setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);
                iniciadoVisualizacion = true;
                buttonPlus.setButtonEnabled(false);
                buttonMinus.setButtonEnabled(false);
                startButton.setButtonEnabled(false);
            }

            if (restartButton.isPressed) {
                restart(iniciadoVisualizacion, indiceOperacion, a, b, c, numDisks, windowWidth, diskHeight, colors, operations, buttonPlus, buttonMinus, startButton, restartButton);
            }
        }

        // Update
        if (iniciadoVisualizacion && indiceOperacion < (int)operations.size()) {
            // Animación
            if (!animating) {
                Operation operation = operations[indiceOperacion];

                std::string statusStr = "[" + std::to_string(indiceOperacion + 1) + "/" + std::to_string(operations.size()) + "]" + " Mover disco " + std::to_string(operation.diskNum) + " de " + std::string(1, operation.sourceLetter) + " a " + std::string(1, operation.destinationLetter);
                currentOperationText.setString(statusStr);
                std::cout << statusStr << std::endl;

                Tower* source = nullptr;
                Tower* destination = nullptr;
                for (Tower* tower : towers) {
                    if (tower->getLetter() == operation.sourceLetter) {
                        source = tower;
                    }
                    if (tower->getLetter() == operation.destinationLetter) {
                        destination = tower;
                    }
                }

                currentSource = source;
                currentDestination = destination;
                currentDisk = &source->getTopDisk();

                animating = true;
                delta = 0.f;
                const float goalX = destination->getPosition().x - currentDisk->getShape().getSize().x / 2;
                const float goalY = destination->getPosition().y - (destination->getDisks().size() + 1) * diskHeight - 10;
                goal = sf::Vector2f(goalX, goalY);
                init = sf::Vector2f(currentDisk->getPosition().x, currentDisk->getPosition().y);
            }

            if (delta < 1.f) {
                delta += 0.01f;
                animateDiskMove(currentSource->getTopDisk(), init, goal, a.getPosition().y - towerHeight - 30, delta);
            } else {
                animating = false;
                moveDisk(*currentSource, *currentDestination, operations, false);
                indiceOperacion++;

                if (indiceOperacion >= (int)operations.size()) {
                    restartButton.setButtonEnabled(true);
                }
            }
        }

        // Draw
        window.clear();

        // - Base
        window.draw(base);
        window.draw(labelA);
        window.draw(labelB);
        window.draw(labelC);

        // - Torres
        for (Tower* tower : towers) {
            // Palo
            sf::RectangleShape palo;
            palo.setSize(sf::Vector2f(towerWidth, towerHeight));
            palo.setFillColor(sf::Color::White);
            palo.setPosition(tower->getPosition().x - towerWidth / 2, tower->getPosition().y - towerHeight);
            window.draw(palo);
        }
        for (Tower* tower : towers) {
            std::stack<Disk> disks = tower->getDisks();
            // Discos
            while (!disks.empty()) {
                Disk disk = disks.top();
                window.draw(disk.getShape());
                sf::Text diskNum(std::to_string(disk.getNum()), buttonFont, 14);
                diskNum.setPosition(
                    disk.getShape().getPosition().x + disk.getShape().getSize().x / 2 - 7,
                    disk.getShape().getPosition().y + disk.getShape().getSize().y / 2 - 7
                );
                diskNum.setFillColor(inverseLegibleColor(disk.getColor()));
                window.draw(diskNum);
                disks.pop();
            }
        }

        // - Botones
        if (!iniciadoVisualizacion) {
            ndisksText.setString("n: " + std::to_string(numDisks) + ", movimientos necesarios: " + std::to_string(numMoves));
            window.draw(ndisksText);
            buttonPlus.draw(window);
            buttonMinus.draw(window);
            startButton.draw(window);
        }

        if (iniciadoVisualizacion) {
            window.draw(currentOperationText);
            restartButton.draw(window);
        }

        window.display();
    }
}

void calculateTowersPos(Tower &a, const float windowWidth, const float windowHeight, float towerHeight, Tower &b, Tower &c, sf::RectangleShape &base, sf::Text &labelA, sf::Text &labelB, sf::Text &labelC) {
    a.setPosition(1 * windowWidth / 4 - 15, windowHeight - (windowHeight - towerHeight) / 2);
    b.setPosition(2 * windowWidth / 4, windowHeight - (windowHeight - towerHeight) / 2);
    c.setPosition(3 * windowWidth / 4 + 15, windowHeight - (windowHeight - towerHeight) / 2);
    base.setPosition(50, towerHeight + (windowHeight - towerHeight) / 2);
    labelA.setPosition(a.getPosition().x - 5, a.getPosition().y + 4);
    labelB.setPosition(b.getPosition().x - 5, b.getPosition().y + 4);
    labelC.setPosition(c.getPosition().x - 5, c.getPosition().y + 4);
}
