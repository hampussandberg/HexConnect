quit -sim

vcom -93 -work work {../../spi_slave_controller.vhd}

vsim work.spi_slave_controller

delete wave *
configure wave -namecolwidth 180
configure wave -valuecolwidth 80
config wave -signalnamewidth 1

add wave -noupdate -divider -height 16 "Clk and reset"
add wave -noupdate clk
add wave -noupdate reset_n

add wave -noupdate -divider -height 16 "Module Interface"
add wave -noupdate -radix hexadecimal data_to_send
add wave -noupdate data_to_send_valid
add wave -noupdate -radix hexadecimal data_received
add wave -noupdate data_received_valid
add wave -noupdate busy_transfer

add wave -noupdate -divider -height 16 "External Hardware Interface"
add wave -noupdate spi_mosi
add wave -noupdate spi_mosi_synced
add wave -noupdate spi_cs_n
add wave -noupdate spi_cs_n_synced
add wave -noupdate spi_cs_n_synced_last
add wave -noupdate spi_sclk
add wave -noupdate spi_sclk_synced
add wave -noupdate spi_sclk_synced_last
add wave -noupdate spi_miso

add wave -noupdate -divider -height 16 "Some Internal signals"
add wave -noupdate bit_count
add wave -noupdate -radix hexadecimal receivedByte
add wave -noupdate -radix hexadecimal transmittedByte



force reset_n 0 0ns, 1 50ns
force -freeze clk 1 0, 0 {5 ns} -r {10 ns}

force spi_sclk 0 0ns, 1 300ns, 0 350ns, 1 400ns, 0 450ns, 1 500ns, 0 550ns, 1 600ns, 0 650ns, 1 700ns, 0 750ns, 1 800ns, 0 850ns, 1 900ns, 0 950ns, 1 1000ns, 0 1050ns
force spi_sclk 1 1350ns, 0 1400ns, 1 1450ns, 0 1500ns, 1 1550ns, 0 1600ns, 1 1650ns, 0 1700ns, 1 1750ns, 0 1800ns, 1 1850ns, 0 1900ns, 1 1950ns, 0 2000ns, 1 2050ns, 0 2100ns
force spi_mosi 1 0ns, 0 1300ns, 1 1700ns
force spi_cs_n 1 0ns, 0 200ns, 1 1100ns, 0 1300ns

force data_to_send_valid 0 0ns, 1 100ns, 0 110ns
force data_to_send "10101010"

run 2500ns

wave zoomfull

