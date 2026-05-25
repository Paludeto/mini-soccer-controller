#include <QCoreApplication>
#include <SDL.h>
#include <QUdpSocket>
#include <QString>
#include <QThread>
#include <vssref_common.pb.h>
#include <packet.pb.h>

using namespace std;

typedef enum {
    BLUE,
    YELLOW
} Color;

class ActuatorClient {
private:
    QUdpSocket *_udpSocket;
    QHostAddress _address;
    quint16 _port;
    quint8 _robotId;
    Color _color;
    float _baseSpeed{32.0f};
    bool _isConnected{false};

public:
    ActuatorClient(QString newAddress, quint16 newPort, Color newColor) {
        _address = QHostAddress(newAddress);
        _port = newPort;
        _udpSocket = new QUdpSocket();
        _udpSocket->connectToHost(_address, _port, QUdpSocket::WriteOnly);

        _isConnected = _udpSocket->isOpen();
        _color = newColor;
    }

    void setRobotId(quint8 newId) {
        _robotId = newId;
    }

    void sendWheelCommand(float leftSpeed, float rightSpeed) {
        fira_message::sim_to_ref::Packet packet;
        fira_message::sim_to_ref::Command *command = packet.mutable_cmd()->add_robot_commands();

        command->set_yellowteam(_color == YELLOW);
        command->set_id(_robotId);
        command->set_wheel_left(leftSpeed);
        command->set_wheel_right(rightSpeed);

        string buffer;
        packet.SerializeToString(&buffer);
        if (_udpSocket->write(buffer.c_str(), buffer.length()) == -1) {
            cout << "[ERROR]\n";
        }
    }
};

namespace Controller {

    const int DEADZONE = 8000;
    const float AXIS_MAX = 32767.0f;

    SDL_GameController* open(int index) {
        if (SDL_IsGameController(index)) {
            SDL_GameController *gc = SDL_GameControllerOpen(index);
            if (gc) {
                cout << "Controle aberto no índice " << index << ": " << SDL_GameControllerName(gc) << endl;
                return gc;
            }
        }
        return nullptr;
    }

    void close(SDL_GameController* gc) {
        if (gc) SDL_GameControllerClose(gc);
    }

    // Returns {leftWheel, rightWheel} from analog stick
   pair<float, float> getWheelSpeeds(SDL_GameController* gc, float baseSpeed) {
        if (!gc) return {0.0f, 0.0f};

        Sint16 ly = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);
        Sint16 lx = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 ry = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTY);
        Sint16 rx = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTX);

        Sint16 rawY = (abs(ry) > abs(ly)) ? ry : ly;
        Sint16 rawX = rx; // somente stick direito para virar

        // Normalize, apply deadzone, square for gentler near-center response
        auto process = [&](Sint16 raw) -> float {
            if (abs(raw) < DEADZONE) return 0.0f;
            float norm = raw / AXIS_MAX;                  // [-1, 1]
            return -norm * norm * norm * baseSpeed;       // quadratic, preserves sign
        };

        float forward = process(rawY);
        float turn    = process(rawX);

        return {forward - turn, forward + turn};
    }   

} // namespace Controller

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    ActuatorClient blue("127.0.0.1", 20013, BLUE);
    blue.setRobotId(0);

    // ActuatorClient yellow("127.0.0.1", 20012, YELLOW);
    // yellow.setRobotId(0);

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) return 1;

    SDL_GameController *gcBlue = Controller::open(0);
    // SDL_GameController *gcYellow = Controller::open(0);

    if (!gcBlue) {
        std::cout << "Controle não detectado!" << std::endl;
        exit(EXIT_FAILURE);
    }

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        auto [blueL, blueR] = Controller::getWheelSpeeds(gcBlue, 32.0f);
        blue.sendWheelCommand(blueL, blueR);

        // auto [yellowL, yellowR] = Controller::getWheelSpeeds(gcYellow, 32.0f);
        // yellow.sendWheelCommand(yellowL, yellowR);

        QThread::msleep(100);
    }

    Controller::close(gcBlue);
    // Controller::close(gcYellow);
    SDL_Quit();
    return 0;
    
}