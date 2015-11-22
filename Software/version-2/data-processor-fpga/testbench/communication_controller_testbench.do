quit -sim

vcom -work work {testbench/communication_controller_testbench.vhd}
vcom -work work {communication_data_manager.vhd}
vcom -work work {spi_slave_controller.vhd}

vsim -t ns work.communication_controller_tb

delete wave *
configure wave -namecolwidth 200
configure wave -valuecolwidth 80
config wave -signalnamewidth 1

add wave -noupdate -divider -height 16 "Clk and reset"
add wave -noupdate clk
add wave -noupdate reset_n

add wave -noupdate -divider -height 16 "Channel ID"
add wave -noupdate channel_id_1
add wave -noupdate channel_id_2
add wave -noupdate channel_id_3
add wave -noupdate channel_id_4
add wave -noupdate channel_id_5
add wave -noupdate channel_id_6
add wave -noupdate channel_id_update

add wave -noupdate -divider -height 16 "Channel control signals"
add wave -noupdate channel_pin_c_output
add wave -noupdate channel_power

add wave -noupdate -divider -height 16 "Internal Signals"
add wave -noupdate comm_data_manager_instance/load_tx_data_ready
add wave -noupdate comm_data_manager_instance/load_tx_data_ready_synced
add wave -noupdate comm_data_manager_instance/load_tx_data
add wave -noupdate comm_data_manager_instance/tx_data
add wave -noupdate comm_data_manager_instance/rx_data_ready
add wave -noupdate comm_data_manager_instance/rx_data
add wave -noupdate comm_data_manager_instance/transfer_in_progress
#add wave -noupdate spi_sclk_enabled
#add wave -noupdate spi_byte_to_send

add wave -noupdate -divider -height 16 "External Hardware Interface"
add wave -noupdate spi_data_ncs
add wave -noupdate spi_data_sclk
add wave -noupdate spi_data_mosi
add wave -noupdate spi_data_miso

add wave -noupdate -divider -height 16 "Extra"
add wave -noupdate spi_slave_instance/tx_next

#add wave -noupdate -divider -height 16 "Debug"
#add wave -noupdate debug_spi_data_ncs
#add wave -noupdate debug_leds


run 6500ns
wave zoomfull

