# ⚽ Soccer Controller

Cliente em C++ para controlar robôs no **[TraveSim](https://github.com/futebol-mini/travesim)**, utilizando **Qt6**, **SDL2** e **protobuf**.

---

## 🚀 Pré-requisitos

Este projeto depende do [TraveSim](https://github.com/futebol-mini/travesim).
Certifique-se de que o TraveSim esteja instalado e funcionando antes de usar o controller.

### Dependências principais

* CMake (>= 3.16)
* Qt6 (Core, Network)
* SDL2
* Protobuf (compilador + headers)
* `vsss_sim_pb_msgs` (submódulo git)

---

## 🐧 Instalação de dependências

### Fedora

```bash
sudo dnf install -y cmake gcc-c++ git \
    qt6-qtbase-devel qt6-qtbase-private-devel \
    qt6-qtbase-gui-devel qt6-qttools-devel \
    SDL2-devel protobuf-devel protobuf-compiler
```

### Ubuntu (22.04+)

```bash
sudo apt update
sudo apt install -y cmake g++ git \
    qt6-base-dev qt6-base-dev-tools \
    libsdl2-dev protobuf-compiler libprotobuf-dev
```

### macOS (Homebrew)

```bash
brew install cmake git protobuf sdl2 qt@6
```

> ⚠️ O `qt@6` não é linkado no PATH por padrão.
> Para o CMake encontrar, adicione:
>
> ```bash
> export CMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
> ```

---

## 👽️ Git Submodules

O projeto utiliza a biblioteca `vsss_sim_pb_msgs` como **submódulo**.

### 💥 Adicionar um submódulo

Na raiz do repositório:

Com HTTPS:

```bash
git submodule add --name vsss_sim_pb_msgs https://github.com/ThundeRatz/vsss_sim_pb_msgs.git proto
```

Com SSH:

```bash
git submodule add --name vsss_sim_pb_msgs git@github.com:ThundeRatz/vsss_sim_pb_msgs.git proto
```

### ✅ Inicializar um submódulo existente

Se você clonou um repositório que já possui submódulos:

* Clonar já com submódulos:

```bash
git clone --recurse-submodules git@github.com:ThundeRatz/travesim_adapters.git
```

* Clonar sem submódulos e inicializar depois:

```bash
git submodule update --init
```

---

## ⚙️ Compilação com CMake

Na raiz do projeto:

```bash
mkdir build && cd build
cmake ..
make -j
```

O CMake já lida com as dependências necessárias:

```cmake
target_link_libraries(soccer-controller
    Qt6::Core
    Qt6::Network
    SDL2::SDL2
    pb_msgs_lib
)
```

---

## ▶️ Execução

Após compilar, rode o controller enquanto o **TraveSim** estiver em execução.
Certifique-se de que o TraveSim está rodando e escutando pacotes UDP (azul -> 127.0.0.1:20013, amarelo -> 127.0.0.1:20012).


Exemplo:

* Controle no índice `0` → robô **azul** (id 0)
* Controle no índice `1` → robô **amarelo** (id 0)

---

## 🎮 Controles

* **Analógico esquerdo** → frente / ré
* **Analógico direito** → girar para esquerda / direita
* Nenhum input → motores desligados (`RELEASED`)

---

## 📌 Observações

* Se os robôs não se moverem, verifique:

  * O endereço e porta configurados.
  * Se o TraveSim está rodando corretamente.
  * Se os pacotes protobuf estão compatíveis com a versão usada no TraveSim.
