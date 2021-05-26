# Example Asset

## Description

This repository holds the source code for the Example Asset project. The Example Asset can be interfaced with an Astronode Module. The project provides an example implementation of the Astronode Serial Commands. The goal is to demonstrate the protocol for Astrocast customers.

Be careful to comply with the requirements and instructions below before compiling the source code and programming your target.

**Warnings:**
This project favours simplicity. The goal is to demonstrate the Astronode protocol. Using the STM32 tools such as CubeIDE and HAL in the way shown here may not be suitable for other applications. Please take care to understand the limitations before reusing it in other projects.

This project is provided without any guarantee, and code is reused at your own risk.

### Folder structure

| Files                        | Content                                                                                        |
|------------------------------|------------------------------------------------------------------------------------------------|
| Core/                        | Folder that contains the source files of the code.                                             |
| Core/astrocast/              | Folder that contains the library provided by Astrocast to communicate with the Astronode.      |
| Core/Src/drivers.c           | Interface between the application and the underlying hardware.                                 |
| Core/Src/main.c              | Simple example application.                                                                    |
| Drivers/                     | The source files provided by the manufacturer of the platform, including CMSIS and STM32 HAL.  |

&nbsp;

---

## How to use it
This code is provided in the form of a STM32CubeIDE Project.

- It can be used as it is and flashed to a STM32 Nucleo-64 development board NUCLEO-L476RG, this method is described [here](#stm32cubeide-project).

- It can be used as a reference to import the library provided by Astrocast in your application, this method is described [here](#import-the-library-to-your-code).


&nbsp;

---


## STM32CubeIDE Project
In this section, we will describe all the steps from cloning the repository to sending your first messages to the Astronode.

### Requirements:
- STM32CubeIDE (version 1.3 or higher).
- STM32 Nucleo-64 development board NUCLEO-L476RG.
- 6 jumper wires.
- Astronode device

&nbsp;

1. The first step is to clone this repository to your local machine. To do so you could use :
    > git clone https://github.com/Astrocast/Example_Asset.git

    Or simply download it as a zip files and extract it where it makes sense for you.

2. Then you will need to import this project to STM32CubeIDE. The installation of this software will not be covered here but you can find some information [online](https://www.st.com/resource/en/user_manual/dm00603964-stm32cubeide-installation-guide-stmicroelectronics.pdf).

    When STM32CubeIDE is properly setup:
    > click **_File_** > **_Open Projects from File System..._**

    In the new window:

    > click **_Directory..._**

    and locate the project you have just cloned.

    Then:

    > click **_Finish_**.

    At this point you should have the new project opened in the **Project Explorer** tab.

3. Once the project is correctly imported to STM32CubeIDE, you can connect the STM32 Nucleo-64 development board to your computer through the ST-LINK USB connector using a USB mini B cable.

    You can check if the board is correctly connected via the LEDs and the [documentation](https://www.st.com/resource/en/user_manual/dm00105823-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf#page=23) of the board.

4. The next step is to connect the STM32 Nucleo-64 development board to the Astronode.

    In this part we will assume that you are using a STM32 Nucleo-64 development board NUCLEO-L476RG and you did not modify any gpio configuration.

    You can find the location for each pin [here](https://www.st.com/resource/en/user_manual/dm00105823-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf#page=28).


    | Nucleo Pin | Astronode Pin | Description          |
    |------------|---------------|----------------------|
    | 3V3        | 3V3           | Supply Voltage       |
    | GND        | GND           | Ground               |
    | PA10 (D2)  | TX            | UART Tx line         |
    | PA9 (D8)   | RX            | UART Rx line         |
    | PA12       | EVENT         | EVENT Pin            |
    | PA11       | RESET         | RESET Pin            |


  **WARNING** Note that it is important to differentiate if you are using an Astronode, a Satellite Development Kit or a Wi-Fi Development Kit.

5. If your are using A Wi-Fi Development Kit:
    > Open **_Core>Srx>main.c_**

    Go to line 36, 37 and 38 and

    > Update **_"my_wifi_ssid"_**, **_"my_wifi_password"_** and **_"my_api_token"_** with your actual credentials.

6. If your are **NOT** using A Wi-Fi Development Kit:
    > Open **_Core>Srx>main.c_**

    and

    > Comment from **line 36 to 39**.


7. You can then:

    > click the hammer icon to **build** the project.

    You should end up with this output :

    ```
    02:24:06 **** Incremental Build of configuration Debug for project Example_Asset ****
    make -j4 all
    [...]
    Finished building target: Example_Asset.elf

    arm-none-eabi-objdump -h -S  Example_Asset.elf  > "Example_Asset.list"
    arm-none-eabi-objcopy  -O binary  Example_Asset.elf  "Example_Asset.bin"
    arm-none-eabi-size   Example_Asset.elf
    text	   data	    bss	    dec	    hex	filename
    23028	    120	   1848	  24996	   61a4	Example_Asset.elf
    Finished building: default.size.stdout

    Finished building: Example_Asset.bin

    Finished building: Example_Asset.list

    02:24:07 Build Finished. 0 errors, 0 warnings. (took 1s.402ms)
    ```

8. Finally you can flash the board with the binary you have just built.

    > click the green play button to **Run** the project on the board.

9. You have the possibility to access some debug logs through the ST-Link port:
    > open the port with the following parameters :

    ```
    Configuration:
     TTY device: /dev/serial/by-id-usb-STMicroelectronics_STM32_STLink_066EFF383034544157074709-if02
     Baudrate: 115200
     Databits: 8
     Flow: none
     Stopbits: 1
     Parity: none
     Local Echo: no
     Timestamps: yes
     Output delay: 0
     Map flags: INLCRNL
    ```

    You should see something like:

    ```
    Start the application...
    Message sent to the Astronode -->
    066D795F776966695F73736964[...]
    Message received from the Astronode <--
    86BE10
    WiFi settings successfully set.
    Message sent to the Astronode -->
    0505000316E3
    Message received from the Astronode <--
    85DD20
    Astronode configuration successfully set.
    ```

When the application starts, a few messages are exchanged between the Nucleo-64 development board and the Astronode to setup the Astronode configuration (see [Power Up Sequence](#power-up-sequence)).

Then the Astronode is ready to receive some payloads. To queue a new payload, simply press the blue button.


&nbsp;

---

## Import the library to your code

In this section, we will assume that you are familiar with how the example is working and you want to import it in your codebase.


&nbsp;

In order to do so you have 5 files to add.

| Files                                     | Content                                                                                                             |
|-------------------------------------------|---------------------------------------------------------------------------------------------------------------------|
| Core/astrocast/astronode_definition.h     | Definition of useful constants used in the library.                                                                 |
| Core/astrocast/astronode_application.h    | Declaration of all the functions relative to the various messages you could send.                                   |
| Core/astrocast/astronode_application.c    | Definition of all the functions relative to the various messages you could send.                                    |
| Core/astrocast/astronode_transport.h      | Declaration of the function responsible for sending the message and receiving the response.                         |
| Core/astrocast/astronode_transport.c      | Definition of all the functions responsible for encoding, decoding, sending the message and receiving the response. |


&nbsp;

Once those files are included in your codebase you can use all the functions declared in **_Core/astrocast/astronode_application.h_** to build and send your messages. Those functions in **_Core/astrocast/astronode_application.c_**	provide an easy way to build and send a message.

Simply call the function you want with the right parameters. It will build the message structure as defined by the application layer protocol and pass it to **_Core/astrocast/astronode_transport.c_**. Here, the message will be prepared respecting the transport layer protocol. Finally, the message will be sent via the **send_astronode_request()** function. This function is defined in the **_Core/Src/drivers.c_** as well as the **is_astronode_character_received()**. Those two functions make the link between the application/transport layers and the physical layer.

>The physical layer will obviously be dependent on the platform on which you are running your application, thus you will have to implement your own functions to send and receive bytes through the UARTs and refer to them in **send_astronode_request()** and **is_astronode_character_received()**.

&nbsp;

---
## Power Up Sequence

This section gives some details on the sequence you should follow to use the Example Asset project.

1. Apply the power to the Nucleo-64 development board.

2. Power up the UART lines.

3. Reset the Astronode through the RESET pin.

4. Clear the RESET event.

5. Send the configuration for the Astronode.

6. _Optional_ :  Save the Astronode configuration.

7. Go into an infinite loop to send/receive messages.
