When I participated in the NUEDC competition during my college years, I developed this set of command-line interfaces to make it easier to adjust and debug algorithm parameters and monitor log information, such as fine-tuning parameters in PID algorithm. The operation logic is similar to that of the Linux command-line interface, but it requires very few resources while still providing complete basic functionality. It can be easily ported to different chips, such as NXP and ESP32. It can also be ported to applications or operating systems without a command-line interface, such as standalone systems and RTOS environments.

How to use:
After porting the code to your project, connect your board via the serial port and enter "cmd". At this point, you can see it:
[LEON]@LINKS:cmd      
-------------------- Cmd Table --------------------
cmd:add    eg:add [parm1] [parm2]
cmd:sub    eg:sub [parm1] [parm2]
cmd:mul    eg:mul [parm1] [parm2]
cmd:div    eg:div [parm1] [parm2]
---------------------------------------------------
These four commands are examples, and their functions are respectively to perform addition, subtraction, multiplication and division operations on two numbers.
For example, the result of using the addition command is as follows:
[LEON]@LINKS:add 1 2
add result 3 
If you have a new command, just write a command function. You can follow the example:
void caclu_add(void){
    int parm1=0,parm2=0;

    if(strlen(token[1])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }
    if(strlen(token[2])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }

    parm1 = atoi(token[1]);
    parm2 = atoi(token[2]);
    cli_printf("add result %d \r\n",parm1+parm2);
}
Then register the command function:
_cmd_table cmd_table[]={
...
};
A specific example project: cli_demo. It demonstrated this function on the ESP32-S3 chip.
