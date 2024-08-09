#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include <iostream>
#include <thread>
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

int main()
{
    const unsigned int FPS = 60;
    int numDisks = 3;
    const float windowWidth = 900;
    const float windowHeight = 600;
    const float towerHeight = windowHeight * 0.8;
    const float towerWidth = 20;
    const float diskHeight = 30;
    std::vector<Operation> operations;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight + 50), "Torre de Hanoi");
    window.setFramerateLimit(FPS);

    Tower a(1 * windowWidth / 4 - 15, windowHeight - (windowHeight - towerHeight) / 2, 'A');
    Tower b(2 * windowWidth / 4, windowHeight - (windowHeight - towerHeight) / 2, 'B');
    Tower c(3 * windowWidth / 4 + 15, windowHeight - (windowHeight - towerHeight) / 2, 'C');

    std::vector<sf::Color> colors = { sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta };

    setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);

    std::vector<Tower*> towers = { &a, &b, &c };

    sf::Font buttonFont;
    buttonFont.loadFromFile("./fonts/Arial.ttf");

    // Gráficos
    sf::RectangleShape base;
    base.setSize(sf::Vector2f(windowWidth - 100, towerWidth + 6));
    base.setFillColor(sf::Color::White);
    base.setPosition(50, towerHeight + (windowHeight - towerHeight) / 2);
    sf::Text labelA("A", buttonFont, 14);
    labelA.setFillColor(sf::Color::Black);
    labelA.setPosition(a.getPosition().x - 5, a.getPosition().y + 4);
    sf::Text labelB("B", buttonFont, 14);
    labelB.setFillColor(sf::Color::Black);
    labelB.setPosition(b.getPosition().x - 5, b.getPosition().y + 4);
    sf::Text labelC("C", buttonFont, 14);
    labelC.setFillColor(sf::Color::Black);
    labelC.setPosition(c.getPosition().x - 5, c.getPosition().y + 4);

    // Controles inicio
    sf::Text ndisksText("n: " + std::to_string(numDisks), buttonFont, 20);
    ndisksText.setPosition(50, towerHeight + 100);
    ndisksText.setFillColor(sf::Color::White);
    RectButton buttonPlus(buttonFont, sf::Vector2f(40.f, 20.f), sf::Vector2f(100.f, towerHeight + 125.f));
    buttonPlus.setButtonLabel(20.f, " + ");
    RectButton buttonMinus(buttonFont, sf::Vector2f(40.f, 20.f), sf::Vector2f(50.f, towerHeight + 125.f));
    buttonMinus.setButtonLabel(20.f, " - ");
    RectButton startButton(buttonFont, sf::Vector2f(190.f, 30.f), sf::Vector2f(windowWidth - 190 - 50, towerHeight + 110.f));
    startButton.setButtonLabel(20.f, "Iniciar visualizacion");
    startButton.setButtonColor(sf::Color(0, 200, 0), sf::Color(0, 150, 0), sf::Color(0, 100, 0));
    startButton.setLabelColor(sf::Color::White);

    // Controles visualizacion
    sf::Text currentOperationText("Operacion: --", buttonFont, 20);
    currentOperationText.setPosition(50, towerHeight + 100);
    currentOperationText.setFillColor(sf::Color::White);
    RectButton restartButton(buttonFont, sf::Vector2f(190.f, 30.f), sf::Vector2f(windowWidth - 190 - 50, towerHeight + 110.f));
    restartButton.setButtonLabel(20.f, "Reiniciar");
    restartButton.setButtonColor(sf::Color(0, 200, 0), sf::Color(0, 150, 0), sf::Color(0, 100, 0));
    restartButton.setLabelColor(sf::Color::White);
    restartButton.setButtonEnabled(false);

    // Estado
    bool iniciadoVisualizacion = false;
    int indiceOperacion = 0;

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
                    setDisks(a, b, c, numDisks, windowWidth, diskHeight, colors);
                }
            }

            if (buttonMinus.isPressed) {
                if (numDisks > 1) {
                    numDisks--;
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
            Operation operation = operations[indiceOperacion];
            
            currentOperationText.setString("[" + std::to_string(indiceOperacion + 1) + "/" + std::to_string(operations.size()) + "]" + " Mover disco " + std::to_string(operation.diskNum) + " de " + std::string(1, operation.sourceLetter) + " a " + std::string(1, operation.destinationLetter));
            std::cout << "[" << indiceOperacion + 1 << "/" << operations.size() << "] Mover disco " << operation.diskNum << " de " << operation.sourceLetter << " a " << operation.destinationLetter << std::endl;

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
            moveDisk(*source, *destination, operations, false);
            indiceOperacion++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            if (indiceOperacion >= (int)operations.size()) {
                restartButton.setButtonEnabled(true);
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
            std::stack<Disk> disks = tower->getDisks();
            // Palo
            sf::RectangleShape palo;
            palo.setSize(sf::Vector2f(towerWidth, towerHeight));
            palo.setFillColor(sf::Color::White);
            palo.setPosition(tower->getPosition().x - towerWidth / 2, tower->getPosition().y - towerHeight);
            window.draw(palo);
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
            ndisksText.setString("n: " + std::to_string(numDisks));
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
