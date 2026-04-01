`timescale 1ns / 1ps

module slot_machine_top(
    input clk,
    input trigger_in,
    input reset_n,
    output reg [2:0] led_s1,
    output reg [2:0] led_s2,
    output reg [2:0] led_s3,
    output reg win_led,
    output s1_to_esp,
    output s2_to_esp,
    output s3_to_esp,
    output win_to_esp
);

    // 1. Clock Divider (ปรับให้เร็วขึ้นเพื่อการสุ่มที่ยากขึ้น)
    reg [21:0] div;
    reg slow_clk;
    always @(posedge clk) begin
        if (div >= 22'd500000) begin // ลดตัวหารลงเพื่อให้เลขวิ่งเร็วขึ้น
            slow_clk <= ~slow_clk;
            div <= 0;
        end else begin
            div <= div + 1;
        end
    end

    // 2. LFSR (สุ่มตลอดเวลา ห้ามใช้ Reset มาหยุดส่วนนี้)
    reg [7:0] r1 = 8'hA1;
    reg [7:0] r2 = 8'h2B;
    reg [7:0] r3 = 8'hFE;
    always @(posedge slow_clk) begin
        r1 <= {r1[6:0], r1[7]^r1[5]}; 
        r2 <= {r2[6:0], r2[7]^r2[3]}; 
        r3 <= {r3[6:0], r3[7]^r3[4]};
    end

    // 3. Edge Detection (กันรัว)
    reg t_sync, t_prev;
    always @(posedge clk) begin
        t_sync <= trigger_in;
        t_prev <= t_sync;
    end
    wire trig_edge = (t_sync && !t_prev); 

    // 4. Step Control (ใช้ Reset เฉพาะส่วนนี้)
    reg [1:0] step;
    always @(posedge clk or negedge reset_n) begin
        if(!reset_n) step <= 0;
        else if(trig_edge) begin
            if (step == 2'd3) step <= 3; // ค้างที่ผลลัพธ์จนกว่าจะ Reset บอร์ด
            else step <= step + 1;
        end
    end

    // 5. ลอจิกหยุดทีละช่อง
    reg [2:0] lock1, lock2, lock3;
    always @(posedge clk) begin
        if (step == 2'd0) begin
            led_s1 <= r1[2:0]; led_s2 <= r2[2:0]; led_s3 <= r3[2:0];
            win_led <= 0;
        end else if (step == 2'd1) begin
            lock1 <= led_s1; // ล็อคช่อง 1
            led_s2 <= r2[2:0]; led_s3 <= r3[2:0];
        end else if (step == 2'd2) begin
            lock2 <= led_s2; // ล็อคช่อง 2
            led_s1 <= lock1; led_s3 <= r3[2:0];
        end else if (step == 2'd3) begin
            lock3 <= led_s3; // ล็อคช่อง 3
            led_s1 <= lock1; led_s2 <= lock2;
            win_led <= (lock1 == lock2 && lock2 == lock3);
        end
    end

    assign s1_to_esp = led_s1[0]; 
    assign s2_to_esp = led_s2[0];
    assign s3_to_esp = led_s3[0];
    assign win_to_esp = win_led;

endmodule
