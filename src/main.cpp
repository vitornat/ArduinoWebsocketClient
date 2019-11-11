#include <Arduino.h>

#include <WebSocketClient.h>
#include <TaskScheduler.h>

#define _PRNT(a) Serial.print(a)
#define _PRNTN(a) Serial.println(a)
#define NETWORK_CONTROLLER ETHERNET_CONTROLLER_W5100
#define SERVER_PORT 8888          //Porta do servidor
#define SERVER_IP "192.168.150.1" //ip do servidor
#define _TASK_SLEEP_ON_IDLE_RUN

using namespace net;

void enviar();

WebSocketClient client;
Scheduler ts;
Task taskEnviar(1UL, TASK_FOREVER, &enviar, &ts, true);

char *msg;

//ler da porta serial as variáveis
void ler(unsigned int *entrada)
{
    while (!Serial.available())
    {
        ; //enquanto não tem nada aguarda
    }

    char buffer[4];
    size_t num_read = Serial.readBytesUntil('\r', buffer, sizeof(buffer) - 1);
    buffer[num_read] = '\0';
    sscanf(buffer, "%d", entrada);
    _PRNT(F(" "));
    _PRNTN(*entrada);
    return;
}

void entradaDados(unsigned int *frequencia, unsigned int *tamanho, unsigned int *qtPckts)
{
    //entrada de dados
    while (true)
    {
        if (*frequencia == 0)
        {
            _PRNT(F("Frequencia (pckts/s):  "));
            ler(frequencia);
        }
        else if (*tamanho == 0)
        {
            _PRNT(F("Tamanho (bytes):  "));
            ler(tamanho);
        }
        else if (*qtPckts == 0)
        {
            _PRNT(F("Qnt pckts:  "));
            ler(qtPckts);
        }
        else
            break;
    }
    _PRNT(F("Freq: "));
    _PRNTN(*frequencia);
    _PRNT(F("Tam: "));
    _PRNTN(*tamanho);
    _PRNT(F("Qt pckts: "));
    _PRNTN(*qtPckts);
    return;
}

void conectar()
{
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    const IPAddress ip(192, 168, 150, 2);
    const IPAddress gw(192, 168, 150, 1);

    // attempt a DHCP connection:
    //if (!Ethernet.begin(mac))
    // {
    // if DHCP fails, start with a hard-coded address:
    // _PRNTln(F("DHCP falhou"));
    Ethernet.begin(mac, ip);
    Ethernet.setGatewayIP(gw);
    //}

    _PRNT(F("IP: "));
    _PRNTN(Ethernet.localIP());
    _PRNT(F("Gtwy: "));
    _PRNTN(Ethernet.gatewayIP());
    if (Ethernet.linkStatus() == LinkOFF)
    {
        _PRNTN(F("Lnk OFF"));
    }

    client.onMessage([](WebSocket &ws, const WebSocketDataType dataType, const char *message, uint16_t length) {
        _PRNTN(message);
    });

    client.onClose([](WebSocket &ws, const WebSocketCloseCode code, const char *reason, uint16_t length) {
        _PRNTN(F("Disconnected"));
    });

    if (!client.open(SERVER_IP, SERVER_PORT))
    {
        _PRNTN(F("Connection failed!"));
        while (true)
            ;
    }
}

void enviar()
{
    _PRNT(F("pckg - "));
    _PRNTN(taskEnviar.getIterations());
    client.send(TEXT, msg, strlen(msg));
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;

    //frequência de envio de mensagens mensagens/s
    unsigned int frequencia = 0;
    //tamanho pré-difinido  da mensagem em Bytes
    unsigned int tamanho = 0;
    //número máximo de pacotes
    unsigned int qtPckts = 0;

    entradaDados(&frequencia, &tamanho, &qtPckts);

    //preechendo o array[]
    msg = new char[tamanho + 1];
    memset(msg, 'C', tamanho + 1);
    msg[tamanho] = '\0';

    conectar();
    taskEnviar.setInterval(1000 / frequencia);
    taskEnviar.setIterations(qtPckts);
    ts.startNow();
} //end setup

void loop()
{
    ts.execute();
} //end loop
