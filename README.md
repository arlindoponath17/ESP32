# ESP32


in VSCode terminal:
source esp-idf-v5.4.1/export.sh
idf.py menuconfig
    -> Example Connection Configuration ---> [*] Connect using Ethernet interface
                                        ---> Ethernet Type (Internal EMAC) ---> (X) Internal EMAC
                                        ---> Ethernet PHY Device (LAN87xx) ---> (X) LAN87xx 
                                        ---> (23) SMI MDC GPIO number
                                        ---> (18) SMI MDIO GPIO number
                                        ---> (-1) PHY Reset GPIO number
                                        ---> (1) PHY Address

    -> Component config ---> Ethernet ---> Support ESP32 internal EMAC controller ---> PHY interface (RMII) ---> (X) Reduced Media Independent Interface (RMII)
                                                                                  |_>  RMII clock mode (Input RMII clock from external) ---> (X) Input RMII clock from external

GPIO_NUM_16 must be set to HIGH before ethernet_init();
