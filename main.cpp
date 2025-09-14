#include <QCoreApplication>
#include <SDL.h>
#include <QUdpSocket>
#include <QString>
#include <QThread>
#include <vssref_common.pb.h>
#include <packet.pb.h>

using namespace std;

typedef enum {
    RELEASED,
    FWD,
    BCKWD,
    LEFT,
    RIGHT
} Instructions;

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

    void sendCommand(Instructions instruction) {
        
        // Cria o packet
        fira_message::sim_to_ref::Packet packet;
        fira_message::sim_to_ref::Command *command = packet.mutable_cmd()->add_robot_commands();

        // Preenche comando com dados
        command->set_yellowteam(_color == YELLOW);
        command->set_id(_robotId);

        switch (instruction) {
            case FWD:    command->set_wheel_left(_baseSpeed);  command->set_wheel_right(_baseSpeed);  break;
            case LEFT:   command->set_wheel_left(-_baseSpeed / 5); command->set_wheel_right(_baseSpeed / 5);  break;
            case RIGHT:  command->set_wheel_left(_baseSpeed / 5);  command->set_wheel_right(-_baseSpeed / 5); break;
            case BCKWD:  command->set_wheel_left(-_baseSpeed); command->set_wheel_right(-_baseSpeed); break;
            case RELEASED:
            default:     command->set_wheel_left(0); command->set_wheel_right(0); break;
        }

        // Serializa, enfia no packet e manda dados pela rede
        string buffer;
        packet.SerializeToString(&buffer);
        if(_udpSocket->write(buffer.c_str(), buffer.length()) == -1) {
            cout << "[ERROR]\n";
        }

    }
}; // classe client

namespace Controller {

    const int DEADZONE = 8000;

    // Inicializa e retorna o handle para um controle
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

    // Fecha um controle
    void close(SDL_GameController* gc) {
        if (gc) SDL_GameControllerClose(gc);
    }

    // Traduz input para instrução
    Instructions getInstruction(SDL_GameController* gc) {
        if (!gc) return RELEASED;

        Sint16 ly = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);
        bool l1   = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        bool r1   = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

        if (l1 ^ r1) {
            return l1 ? LEFT : RIGHT;
        } else {
            if (ly < -DEADZONE)      return FWD;
            else if (ly > DEADZONE)  return BCKWD;
        }
        return RELEASED;
    }

} // namespace Controller

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    ActuatorClient blue("127.0.0.1", 20013, BLUE);
    ActuatorClient yellow("127.0.0.1", 20012, YELLOW);
    blue.setRobotId(0);
    yellow.setRobotId(0);

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) return 1;

    SDL_GameController *gcBlue   = Controller::open(0);
    SDL_GameController *gcYellow = Controller::open(1);

    if (!gcBlue || !gcYellow) {
        std::cout << "Dois controles não detectados!" << std::endl;
        exit(EXIT_FAILURE);
    }

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        blue.sendCommand(Controller::getInstruction(gcBlue));
        yellow.sendCommand(Controller::getInstruction(gcYellow));

        QThread::msleep(100);
    }

    Controller::close(gcBlue);
    Controller::close(gcYellow);
    SDL_Quit();
    return 0;

} // main