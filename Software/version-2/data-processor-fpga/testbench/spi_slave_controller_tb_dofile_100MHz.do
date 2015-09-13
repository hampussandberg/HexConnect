quit -sim

vcom -93 -work work {../../spi_slave_controller.vhd}

vsim -t ns work.spi_slave_controller

delete wave *
configure wave -namecolwidth 200
configure wave -valuecolwidth 80
config wave -signalnamewidth 1

add wave -noupdate -divider -height 16 "Clk and reset"
add wave -noupdate reset_n

add wave -noupdate -divider -height 16 "Module Interface"
add wave -noupdate transfer_in_progress
add wave -noupdate load_tx_data_ready
add wave -noupdate load_tx_data
add wave -noupdate tx_data
add wave -noupdate rx_data_ready
add wave -noupdate rx_data


add wave -noupdate -divider -height 16 "External Hardware Interface"
add wave -noupdate spi_cs_n
add wave -noupdate spi_sclk
add wave -noupdate spi_mosi
add wave -noupdate spi_miso

add wave -noupdate -divider -height 16 "Some Internal signals"
add wave -noupdate bit_count
add wave -noupdate tx_buffer
add wave -noupdate rx_buffer
#add wave -noupdate load_tx_data_ready_internal
#add wave -noupdate rx_data_ready_internal



force reset_n 0 0ns, 1 20ns

# Test 1: single byte tx/rx, notice simulated clock error at 221ns
force spi_sclk 0 0ns, 1 200ns, 0 210ns, 1 221ns, 0 230ns, 1 240ns, 0 250ns, 1 260ns, 0 270ns, 1 280ns, 0 290ns, 1 300ns, 0 310ns, 1 320ns, 0 330ns, 1 340ns, 0 350ns
force spi_cs_n 1 0ns, 0 100ns, 1 400ns
force spi_mosi 0 0ns, 1 190ns, 0 230ns, 1 270ns, 0 310ns, 1 330ns

force load_tx_data 0 0ns, 1 43ns, 0 47ns
force tx_data "00000000" 0ns, "01010011" 40ns


# Test 2: Two bytes tx/rx
force spi_sclk 1 510ns, 0 520ns, 1 530ns, 0 540ns, 1 550ns, 0 560ns, 1 570ns, 0 580ns, 1 590ns, 0 600ns, 1 610ns, 0 620ns, 1 630ns, 0 640ns, 1 650ns, 0 660ns
force spi_sclk 1 670ns, 0 680ns, 1 690ns, 0 700ns, 1 710ns, 0 720ns, 1 730ns, 0 740ns, 1 750ns, 0 760ns, 1 770ns, 0 780ns, 1 790ns, 0 800ns, 1 810ns, 0 820ns
force spi_cs_n 0 500ns, 1 830ns
force spi_mosi 0 660ns, 1 740ns

force load_tx_data 1 467ns, 0 470ns, 1 664ns, 0 670ns
force tx_data "11110101" 467ns, "11000011" 664ns


run 1000ns
wave zoomfull

