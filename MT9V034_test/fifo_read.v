`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    09:07:11 07/19/2016 
// Design Name: 
// Module Name:    fifo_read 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module fifo_read(
	input reset_pointer,
	input get_data, // from microblaze (sent to trigger new read from FIFO to FPGA buffer)
	input [9:0] pixel_addr, // from microblaze, 0-751
	input [7:0] fifo_data, // 8 bit data in from fifo
	input fifo_rck, // 1MHz clock signal generated by FPGA
	output reg fifo_rrst, // fifo read reset (reset read addr pointer to 0)
	output reg fifo_oe, // fifo output enable (allow for addr pointer to increment)
	output reg buffer_ready,
	output [7:0] pixel_value, // 8-bit value from internal buffer
	output [15:0] current_line
   );

parameter [1:0] ready = 2'b00;
parameter [1:0] read = 2'b01;
parameter [1:0] done = 2'b10;  
parameter [1:0] init = 2'b11;

reg [1:0] state = ready;
reg [1:0] prev_state, next_state = ready;

reg [7:0] pixel_line [0:751]; // implemented in BRAM
reg [9:0] pixel = 10'b00_0000_0000;
reg [15:0] num_lines = 16'h0000;

always @(posedge fifo_rck)
	state <= next_state;
	
always @(state,get_data,pixel)	
	case(state)
		ready: 
			begin
				if(get_data) 
					next_state = init;
				else
					next_state = ready;
					
				prev_state = ready;
			end
		init: 
			begin
				next_state = read;
				prev_state = init;
			end
		read: 
			begin
				if(pixel == 751)
					next_state = done;
				else
					next_state = read;
					
				prev_state = read;
			end
		done: 
			begin
				next_state = ready;
				prev_state = done;
			end
	endcase

always @(posedge fifo_rck)
begin
		if(reset_pointer) 
			begin
			fifo_rrst <= 1'b0;
			num_lines <= 16'h0000;
			end
		else if(state==ready) // allow for MCS to read from pixel_line
			begin
			//pixel_value [7:0] <= pixel_line[pixel_addr];
			fifo_rrst <= 1'b1; // make sure read addr doesn't get reset
			end
		else if(state == init) // prepare to read new data from the AL422 into pixel_line
			begin
			pixel <= 10'b00_0000_000;
			num_lines <= num_lines + 1'b1;
			buffer_ready <= 1'b0;
			fifo_oe <= 1'b0; // allow for read pointer to increment
			end
		else if(state == read) // read data in from the AL422
			begin
			if(next_state == done)
				fifo_oe <= 1'b1; // turn off read enable
			if(prev_state != init) // one cycle delay between init and valid data
			begin
				pixel_line[pixel] <= fifo_data;
				pixel <= pixel + 1'b1;
			end
			end
		else if(state == done)
			begin
			buffer_ready <= 1'b1;
			end
end

// display number of lines written on 7seg display
assign current_line = (num_lines); 
// allow for MCS to read stored pixel line at given addr if state==ready
assign pixel_value [7:0] = pixel_line[pixel_addr];

endmodule
