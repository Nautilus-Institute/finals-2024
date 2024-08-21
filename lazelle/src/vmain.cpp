#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#include "Vamber_wrapper.h"

char gstring[80];
char newline[1024];
unsigned char hexline[1024];

#define SRAMMASK  0x00000FFF
#define RAMBASE  0x00010000
#define LUASTART 0x10000000
#define FLAGTABLE (LUASTART - 0x1000000)
#define LUAINST  (RAMBASE + 0x10000)
#define RAMMASK  0x3FFFFFFF
uint32_t *ram = NULL;
uint32_t *sram = NULL;
uint32_t flag[0x100];

#define FUNCINFO (startaddr + 0x0)
#define LOCALS (startaddr + 0x8000)
#define UPVALS (startaddr + 0x8200)
#define CONSTANTS (startaddr + 0x10000)
#define CONSTANTDATA (startaddr + 0x18000)
#define PROTOS (startaddr + 0x20000)
#define INSTRUCTIONS (instaddr)
#define ENDOFFSET (startaddr + 0x30000)
#define USER_INPUT_TABLE (LUASTART - 0x100000)
#define BUILTIN_ENV_RAW_TABLE (LUASTART - 0x30000)

struct {
    uint32_t src;
    uint32_t dst;
    uint32_t cfg;
    uint32_t sz; // this is size >> 2
    uint32_t direction;
    uint32_t op;
    uint32_t count;
    uint32_t enable;
} dma_obj;

#ifdef DISTRIB
const char * ctrlstate_lookup(int id) {

    const char *states[] = {
        "RST_WAIT1", // 0
        "RST_WAIT2", // 1
        "INT_WAIT1", // 2
        "INT_WAIT2", // 3
        "EXECUTE", // 4
        "PRE_FETCH_EXEC", // 5
        "MEM_WAIT1", // 6 
        "MEM_WAIT2", // 7
        "PC_STALL1", // 8
        "PC_STALL2", // 9
        "MTRANS_EXEC1", // 10
        "MTRANS_EXEC2", // 11
        "MTRANS_EXEC3", // 12
        "MTRANS_EXEC3B", // 13
        "MTRANS_EXEC4", // 14
        "MTRANS5_ABORT", // 15
        "MULT_PROC1", // 16
        "MULT_PROC2", // 17
        "MULT_STORE", // 18
        "MULT_ACCUMU", // 19
        "SWAP_WRITE", // 20
        "SWAP_WAIT1", // 21
        "SWAP_WAIT2", // 22
        "COPRO_WAIT", // 23
        "LUA_DECODE", // 24
        "LUA_LOAD1", // 25
        "LUA_LOAD1F", // 26
        "LUA_LOAD2", // 27
        "LUA_LOAD2F", // 28
        "LUA_EXEC", // 29
        "LUA_STOREA", // 30
        "LUA_STOREA2", // 31
        "LUA_PC_STALL", // 32
        "LUA_PC_STALL2", // 33
    };
    if (id >= sizeof(states)/sizeof(states[0])) {
        printf("id %d out of range\n", id);
        exit(0);
    }
    return states[id];
}
#endif

void perform_dma() {
    if (!dma_obj.enable) {
        // somehow we are not enabled
        return;
    }

    // identify the source and dest
    uint32_t src = dma_obj.direction ? dma_obj.dst : dma_obj.src;
    uint32_t dst = dma_obj.direction ? dma_obj.src : dma_obj.dst;

    uint32_t adjusted_sz = dma_obj.sz << 2;

#ifdef DISTRIB
    printf("dma executing 0x%x -> 0x%x | sz: 0x%x count: 0x%x op: 0x%x\n", src, dst, adjusted_sz, dma_obj.count, dma_obj.op);
#endif

    uint32_t curr_src = 0;
    uint32_t curr_dst = 0;

    for(uint32_t x = 0; x < dma_obj.count; x++) {
        for(uint32_t y = 0; y < adjusted_sz; y+=4) {
            switch (dma_obj.op) {
            case 0:
                curr_src = src + (y << 2) + (x * adjusted_sz);
                curr_dst = dst + (y << 2) + (x * adjusted_sz);
                curr_src &= RAMMASK;
                curr_dst &= RAMMASK;
#ifdef DISTRIB
                printf("dma write ram[0x%X] = ram[0x%X] (prev: 0x%08x new: 0x%08x)\n", RAMBASE + curr_dst, RAMBASE + curr_src, ram[curr_dst>>2], ram[curr_src>>2]);
#endif
                ram[curr_dst>>2] = ram[curr_src>>2];
                break;
            case 1:
                curr_src = src + (y << 2);
                curr_dst = dst + (y << 2) + (x * adjusted_sz);
                curr_src &= RAMMASK;
                curr_dst &= RAMMASK;
#ifdef DISTRIB
                printf("dma write-repeat ram[0x%X] = ram[0x%X] (prev: 0x%08x new: 0x%08x)\n", RAMBASE + curr_dst, RAMBASE + curr_src, ram[curr_dst>>2], ram[curr_src>>2]);
#endif
                ram[curr_dst>>2] = ram[curr_src>>2];
                break;
            default:
                // do nothing....
                printf("unknown dma operation %x\n", dma_obj.op);
            }
        }
    }
    dma_obj.enable = 0;
}

int load_sram ()
{

unsigned int addhigh;
unsigned int add;

unsigned int ra;

unsigned int line;

unsigned char checksum;

unsigned int len;
unsigned int hexlen;
unsigned int maxadd;

unsigned char t;

    FILE *fp = fopen("./src/rom.hex", "rt");
    if (fp == NULL) {
        printf("failed to open rom, this is fatal.\n");
        exit(1);
    }

    maxadd=0;

    addhigh=0;
    memset(sram, 0xFF, 0x1000);

    line=0;
    while(fgets(newline,sizeof(newline)-1,fp))
    {
        line++;
        if(newline[0]!=':')
        {
            printf("Syntax error <%u> no colon\n",line);
            continue;
        }
        gstring[0]=newline[1];
        gstring[1]=newline[2];
        gstring[2]=0;
        len=strtoul(gstring,NULL,16);
        for(ra=0;ra<(len+5);ra++)
        {
            gstring[0]=newline[(ra<<1)+1];
            gstring[1]=newline[(ra<<1)+2];
            gstring[2]=0;
            hexline[ra]=(unsigned char)strtoul(gstring,NULL,16);
        }
        checksum=0;
        for(ra=0;ra<(len+5);ra++) checksum+=hexline[ra];
        //checksum&=0xFF;
        if(checksum)
        {
            printf("checksum error <%u>\n",line);
        }
        add=hexline[1]; add<<=8;
        add|=hexline[2];
        add|=addhigh;
        if(add>SRAMMASK)
        {
            printf("address too big 0x%08X\n",add);
            //return(1);
            continue;
        }
        if(add&3)
        {
            printf("bad address 0x%08X\n",add);
            return(1);
        }
        t=hexline[3];
        if(t!=0x02)
        {
            if(len&3)
            {
                printf("bad length\n");
                return(1);
            }
        }

        switch(t)
        {
            default:
                printf("UNKOWN type %02X <%u>\n",t,line);
                break;
            case 0x00:
                for(ra=0;ra<len;ra+=4)
                {
                    if(add>SRAMMASK)
                    {
                        printf("address too big 0x%08X\n",add);
                        break;
                    }
                    sram[add>>2]                  =hexline[ra+4+3];
                    sram[add>>2]<<=8; sram[add>>2]|=hexline[ra+4+2];
                    sram[add>>2]<<=8; sram[add>>2]|=hexline[ra+4+1];
                    sram[add>>2]<<=8; sram[add>>2]|=hexline[ra+4+0];
                    add+=4;
                    if(add>maxadd) maxadd=add;
                }
                break;
            case 0x01:
                printf("End of data\n");
                break;
            case 0x02:
                addhigh=hexline[5];
                addhigh<<=8;
                addhigh|=hexline[4];
                addhigh<<=16;
        }
    }

    fclose(fp);
    return(0);
}

int map_lua_func(uint32_t *buf, uint32_t startaddr, uint32_t instaddr, uint32_t *outinstaddr, uint32_t *outidx) {
    uint32_t bufidx = 0;

    uint32_t outinst = INSTRUCTIONS;

    // this may change later
    uint32_t endoffset = ENDOFFSET;

#ifdef DISTRIB
    printf("entered map_lua_func(%p, 0x%x)\n", buf, startaddr);
#endif
    // read in instructions
    uint32_t sz = buf[bufidx++];
#ifdef DISTRIB
    printf("\tinst bufidx - %x / sz - %x\n", bufidx, sz);
#endif
    memcpy(&ram[INSTRUCTIONS >> 2], &buf[bufidx], sz);
    bufidx += (sz >> 2);
    outinst += sz;

    // copy in upvalues
    sz = buf[bufidx++];
#ifdef DISTRIB
    printf("\tupval bufidx - %x / sz - %x\n", bufidx, sz);
#endif
    memcpy(&ram[UPVALS >> 2], &buf[bufidx], sz);
    bufidx += (sz >> 2);
    
    // copy in constant
    sz = buf[bufidx++];
    uint32_t num_constants = sz >> 2;
#ifdef DISTRIB
    printf("\tconst bufidx - %x / sz - %x\n", bufidx, sz);
#endif
    memcpy(&ram[CONSTANTS >> 2], &buf[bufidx], sz);
    bufidx += (sz >> 2);

    // get size of constant data
    sz = buf[bufidx++] >> 2;

    // fix up the constant refs
    uint32_t end = bufidx + sz;
    uint32_t cdoff = CONSTANTDATA;
    uint32_t constant_idx = 0;
    while (bufidx < end) {

        // read the next constantdata
        uint32_t nextsz = buf[bufidx++];
#ifdef DISTRIB
        printf("\tconstant @ 0x%x -> 0x%x (%x byte(s)) = 0x%x [...]\n", ram[(CONSTANTS >> 2) + constant_idx], cdoff, nextsz, buf[bufidx]);
#endif
        memcpy(&ram[cdoff >> 2], &buf[bufidx], nextsz);
        bufidx += (nextsz >> 2);

        ram[(CONSTANTS >> 2) + constant_idx] = cdoff;
        constant_idx++;
        cdoff += nextsz;
    }

    // copy in protos
    uint32_t numprotos = buf[bufidx++];
    for(uint32_t x = 0; x < numprotos; x++) {
        ram[(PROTOS >> 2) + x] = endoffset;
        uint32_t outidx = 0;
        uint32_t innersz = buf[bufidx++] >> 2;
        uint32_t dbg_endoffset = endoffset;
        endoffset = map_lua_func(&buf[bufidx], endoffset, outinst, &outinst, &outidx);
        if (innersz != outidx) {
            printf("corruption, outidx (0x%x) != innersz (0x%x)\n", outidx, innersz);
            exit(1);
        }
#ifdef DISTRIB
        printf("\tproto @ 0x%x = 0x%x (%x byte(s))\n", PROTOS + (x << 2), dbg_endoffset, buf[bufidx]);
#endif
        bufidx += outidx;
    }

    uint32_t maxstacksize = buf[bufidx++];
    uint32_t sizeupvals = buf[bufidx++];

    ram[(FUNCINFO >> 2)] = instaddr;
    ram[(FUNCINFO >> 2) + 1] = UPVALS;
    ram[(FUNCINFO >> 2) + 2] = maxstacksize << 2;
    ram[(FUNCINFO >> 2) + 3] = CONSTANTS;
    ram[(FUNCINFO >> 2) + 4] = PROTOS;
    ram[(FUNCINFO >> 2) + 5] = sizeupvals << 2;
#ifdef DISTRIB
    printf("\tinsts: 0x%x upvals: 0x%x mss: 0x%x constants: 0x%x protos: 0x%x sizeupvals: 0x%x\n",
        ram[(FUNCINFO >> 2)], ram[(FUNCINFO >> 2) + 1], ram[(FUNCINFO >> 2) + 2], ram[(FUNCINFO >> 2) + 3],
        ram[(FUNCINFO >> 2) + 4], ram[(FUNCINFO >> 2) + 5]);
#endif

    if (outidx)
        *outidx = bufidx;
    if (outinstaddr)
        *outinstaddr = outinst;

    return endoffset;
}

int map_builtin_program(uint8_t *userdata, uint32_t length) {

    int fd = open("/chal/builtin.leg", O_RDONLY);

    // read in entire file
    off_t filesz = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    uint32_t *buf = (uint32_t *)calloc(filesz, 1);

    if (read(fd, buf, filesz) < filesz) {
        printf("got truncated builtin, exiting.\n");
        exit(1);;
    }
    close(fd);

    // start parsing from top level function
    uint32_t res = map_lua_func(buf, LUASTART, LUAINST, NULL, NULL);

    // for the first function, generate our _ENV table
    uint64_t env = BUILTIN_ENV_RAW_TABLE;
    ram[(env >> 2)] = (env + 4);

    uint64_t env_ptr = LUASTART - 0x4000;
    ram[(env_ptr >> 2)] = env;

    // insert it into the first function's upval
    uint64_t base_upvals = LUASTART - 0x8000;
    ram[(base_upvals >> 2)] = env_ptr;

    // write it to scratch memory so the core knows where it is
    ram[0x18000000 >> 2] = base_upvals;

    // insert the user input in the form of a table of ints
    uint32_t usertable = USER_INPUT_TABLE;
    ram[(usertable >> 2)] = (usertable + 4);
    ram[((env + 4) >> 2)] = usertable;

    for(uint32_t x = 0; x < length; x += 4) {
        // create an int
        uint32_t intbase = LUASTART - 0x80000 + x;
        ram[(intbase >> 2)] = *(uint32_t *)&userdata[x];
        ram[(usertable >> 2) + 1 + (x >> 2)] = intbase;
    }

    // do the same now, but for the flag
    uint32_t flagtable = LUASTART - 0xc0000;
    ram[(flagtable >> 2)] = (flagtable + 4);
    ram[((env + 4) >> 2) + 1] = flagtable;

    for(uint32_t x = 0; x < length; x++) {
        // create an int
        ram[(flagtable >> 2) + 1 + x] = 0xFFF00000 + (x * 4);
    }

    free(buf);

    ram[(0x18f00018 >> 2)] = 4;

    return 0;
}

uint32_t newtable(uint32_t *base) {
    uint32_t baseval = *base;
    ram[(baseval >> 2)] = (baseval + 4);
    *base = (*base + 0x410);
    return baseval;
}

int map_lua(uint32_t *buf, uint32_t length) {

    // start parsing from top level function
    uint32_t res = map_lua_func(buf, LUASTART, LUAINST, NULL, NULL);

    // for the first function, generate our _ENV table
    uint64_t env = LUASTART - 0x30000;
    ram[(env >> 2)] = (env + 4);

    uint64_t env_ptr = LUASTART - 0x4000;
    ram[(env_ptr >> 2)] = env;

    // insert it into the first function's upval
    uint64_t base_upvals = LUASTART - 0x8000;
    ram[(base_upvals >> 2)] = env_ptr;

    // write it to scratch memory so the core knows where it is
    ram[0x18000000 >> 2] = base_upvals;

    uint32_t flagtable = LUASTART - 0xc0000;
    ram[(flagtable >> 2)] = (flagtable + 4);
    for(uint32_t x = 0; x < length; x++) {
        // create an int
        ram[(flagtable >> 2) + 1 + x] = 0xFFF00000 + (x * 4);
    }

    // create flag table
    uint32_t start = FLAGTABLE;
    uint32_t dummytable = newtable(&start);

    // uint32_t emptytable = newtable(&start);

    for(int count = 0; count < 0x10; count++) {
        for(uint32_t x = 0; x < 0x100; x++) {
            // create an int
            ram[(dummytable >> 2) + 1 + x] = newtable(&start);
        }

        // insert flag
        uint8_t *flagptr = (uint8_t *)flag;
        ram[(dummytable >> 2) + 1 + flagptr[count]] = flagtable;

        // go back out one layer
        flagtable = dummytable;
        dummytable = newtable(&start);
    }
    dummytable = flagtable;
    
    ram[((env + 4) >> 2) + 1] = dummytable;

#ifdef DISTRIB
    printf("lua data ranges from 0x%x -> 0x%x (0x%x byte(s) total)\n", LUASTART, res, res-LUASTART);
#endif
    // finally, write a return address in our activation frame stack
    // so the program actually resets on completion.
    ram[(0x18f00018 >> 2)] = 4;

    return 0;
}

void dump_env() {
    uint32_t env = BUILTIN_ENV_RAW_TABLE + 4;

    // dump up to 0x40 entries
    for(uint32_t x = 0; x < 0x40; x++) {
        uint32_t val = ram[(env >> 2) + x];
        if (val == 0)
            break;

        if ((val>=RAMBASE)&&(val<=(RAMBASE+RAMMASK))) {
            uint32_t target = val & RAMMASK;
            target = ram[(target >> 2)];
            printf("[0x%02x] 0x%x -> 0x%x\n", x, val, target);
        } else {
            printf("[0x%02x] 0x%x -> ?\n", x, val);
        }

    }
}

uint64_t simulate() {

    uint64_t lasttick = 0;
    uint32_t tick = 0;
    uint32_t addr = 0;
    uint32_t mask = 0;
    uint32_t simhalt = 0;
    uint32_t did_reset = 0;
    uint32_t timer_control = 0;
    uint32_t timer_status = 0;
    uint32_t timer0_tick = 0;
    uint32_t timer1_tick = 0;
    uint32_t timer1_match = 0;

    Vamber_wrapper *top = new Vamber_wrapper;

    top->i_system_rdy = 0;
    simhalt=0;
    did_reset=0;
    tick=0;
    timer0_tick=0;
    timer1_tick=0;
    timer1_match=0xFFFFFFFF;
    timer_control=0;
    timer_status=0;
    lasttick=tick;
    while (!Verilated::gotFinish())
    {

        top->i_wb_dat=0;
        top->i_wb_ack=0;

        top->uart_char = 0x00;
        top->uart_char_strobe = 0;
        top->test_out = 0x00000000;
        top->test_out_strobe = 0;

        tick++;
        if(tick<lasttick) printf("tick rollover\n");
        lasttick=tick;

        if((tick&1)==0)
        {
            timer0_tick++;
            if(timer_control&0x0002)
            {
                if(timer1_tick>=timer1_match)
                {
                    timer_status|=0x00000C00;
                    timer1_tick=0x00000000;
                }
                else
                {
                    timer1_tick++;
                }
            }
            else
            {
                timer1_tick++;
            }
        }

        top->i_irq=0;
        if((timer_control&timer_status)&0x400) top->i_irq=1;
        top->i_firq=0;
        if((timer_control&timer_status)&0x800) top->i_firq=1;

        if(did_reset)
        {
#ifdef DISTRIB
            printf("addr: 0x%08x ctrl state - %s o_generic - %lx\n", top->dbg_instruction_address, ctrlstate_lookup(top->dbg_o_control_state), top->dbg_generic);
#endif
            if(top->o_wb_cyc)
            {
                if(top->o_wb_sel)
                {
                    addr=top->o_wb_adr;
#ifdef DISTRIB
                    printf("bus tx: %s(0x%x)\n", top->o_wb_we ? "write" : "read", addr);
#endif
                    if(addr<=SRAMMASK)
                    {
                        if(top->o_wb_we)
                        {
                            sram[addr>>2]=top->o_wb_dat;
                        }
                        else
                        {
                            top->i_wb_dat=sram[addr>>2];
#ifdef DISTRIB
                            printf("read sram[0x%X]=0x%08X\n",addr,sram[addr>>2]);
#endif
                        }
                    }
                    else if((addr>=RAMBASE)&&(addr<=(RAMBASE+RAMMASK)))
                    {
                        addr&=RAMMASK;
                        if(top->o_wb_we)
                        {
                            //write ram
                            if((tick&1)==0)
                            {
                                if(top->o_wb_sel==0x0F)
                                {
                                    //all lanes on, just write
#ifdef DISTRIB
                                    printf("write ram[0x%X] = 0x%08X (prev: 0x%08x)\n", addr, top->o_wb_dat, ram[addr>>2]);
#endif
                                    ram[addr>>2]=top->o_wb_dat;
                                }
                                else
                                {
                                    //read-modify-write
                                    mask=0;
                                    if(top->o_wb_sel&1) mask|=0x000000FF;
                                    if(top->o_wb_sel&2) mask|=0x0000FF00;
                                    if(top->o_wb_sel&4) mask|=0x00FF0000;
                                    if(top->o_wb_sel&8) mask|=0xFF000000;
                                    ram[addr>>2]&=~mask;
                                    ram[addr>>2]|=top->o_wb_dat&mask;
#ifdef DISTRIB
                                    printf("read-modify-write ram[0x%X]=0x%08X\n",addr,ram[addr>>2]);
#endif
                                }
                            }
                        }
                        else
                        {
                            //read ram
                            top->i_wb_dat=ram[addr>>2];
                            if((tick&1)==0)
                            {
#ifdef DISTRIB
                                printf("read  ram[0x%X]=0x%08X\n",addr,ram[addr>>2]);
#endif
                            }
                        }
                    }
                    else
                    {
                        //peripherals
                        if ((addr & 0xFFFFFFF0) == 0xC0000000) {
                            // DMA peripheral
                            if (addr == 0xC0000000) {
                                // src addr
                                if(top->o_wb_we) {
                                    if ((tick & 1) == 0) {
#ifdef DISTRIB
                                        printf("dma src = 0x%x\n", top->o_wb_dat);
#endif
                                        dma_obj.src = top->o_wb_dat;
                                    }
                                } else {
                                    top->i_wb_dat = dma_obj.src;
                                }
                            } else if (addr == 0xC0000004) {
                                // dst addr
                                if(top->o_wb_we) {
                                    if ((tick & 1) == 0) {
#ifdef DISTRIB
                                        printf("dma dst = 0x%x\n", top->o_wb_dat);
#endif
                                        dma_obj.dst = top->o_wb_dat;
                                    }
                                } else {
                                    top->i_wb_dat = dma_obj.dst;
                                }
                            } else if (addr == 0xC0000008) {
                                // count
                                if(top->o_wb_we) {
                                    if ((tick & 1) == 0) {
#ifdef DISTRIB
                                        printf("dma count = 0x%x\n", top->o_wb_dat);
#endif
                                        dma_obj.count = top->o_wb_dat & 0xff;
                                    }
                                } else {
                                    top->i_wb_dat = dma_obj.count;
                                }
                            } else if (addr == 0xC000000c) {
                                // dma config
                                if(top->o_wb_we) {
                                    if ((tick & 1) == 0) {
#ifdef DISTRIB
                                        printf("dma cfg = 0x%x\n", top->o_wb_dat);
#endif
                                        dma_obj.cfg = top->o_wb_dat;

                                        // parse out fields
                                        dma_obj.enable = dma_obj.cfg & 1;
                                        dma_obj.direction = (dma_obj.cfg >> 1) & 1;
                                        dma_obj.op = (dma_obj.cfg >> 4) & 0xf;
                                        dma_obj.sz = (dma_obj.cfg >> 8) & 0xffff;

                                        // enable bit never appears on
                                        dma_obj.cfg &= 0xfffffffe;
                                        if (dma_obj.enable & 1) {
                                            perform_dma();
                                        }
                                    }
                                } else {
                                    top->i_wb_dat = dma_obj.cfg;
                                }
                            }
                        }

                        if ((addr & 0xFFFFFC00) == 0xFFF00000) {
                            // FLAG
                            uint32_t offset = addr - 0xFFF00000;
                            if (offset < 0x100) {
                                // flag
                                if(top->o_wb_we) {
                                    // flag is immutable
                                } else {
                                    top->i_wb_dat = flag[(offset & 0xfc) >> 2];
                                }
                            }
                        }

                        //dummy uart tx register
                        if(addr==0xD0000000)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    top->uart_char=top->o_wb_dat&0xFF;
                                    top->uart_char_strobe=1;
                                }
                            }
                        }
                        //timer starts here
                        if(addr==0xD1000000)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    printf("cant write timer0 tick\n");
                                }
                            }
                            else
                            {
                                top->i_wb_dat=timer0_tick;
                            }
                        }
                        if(addr==0xD1000004)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    printf("cant write timer1 tick\n");
                                }
                            }
                            else
                            {
                                top->i_wb_dat=timer1_tick;
                            }
                        }
                        if(addr==0xD1000014)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    timer1_match=top->o_wb_dat;
                                    //printf("write timer1 match 0x%08X\n",timer1_match);
                                }
                            }
                            else
                            {
                                top->i_wb_dat=timer1_match;
                            }
                        }
                        if(addr==0xD1000020)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    timer_control=top->o_wb_dat;
                                    //printf("write timer control 0x%08X\n",timer_control);
                                    if(timer_control&0x0002)
                                    {
                                        timer1_tick=0x00000000;
                                        timer_status&=~0xC00;
                                    }
                                }
                            }
                            else
                            {
                                top->i_wb_dat=timer_control;
                            }
                        }
                        if(addr==0xD1000024)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    timer_status&=~top->o_wb_dat;
                                }
                            }
                            else
                            {
                                top->i_wb_dat=timer_status;
                            }
                        }
                        //debug register
                        if(addr==0xE0000000)
                        {
                            if(top->o_wb_we)
                            {
                                if((tick&1)==0)
                                {
                                    printf("show 0x%08X\n",top->o_wb_dat);
                                    top->test_out = top->o_wb_dat;
                                    top->test_out_strobe = 1;
                                }
                            }
                        }
                        //stop simulation
                        if(addr==0xF0000000)
                        {
                            printf("simhalt invoked\n");
                            simhalt=1;
                        }
                    }
                    top->i_wb_ack=1;
                }
            }
        }
        else
        {
            if (tick > 11)
            {
                top->i_system_rdy = 1;
                did_reset = 1;
            }
        }

        top->timer0_count = timer0_tick;
        top->timer1_count = timer1_tick;
        top->timer1_match = timer1_match;
        top->timer_status = timer_status;
        top->timer_control = timer_control;



        top->i_clk = (tick & 1);
        top->eval();
        if(tick>100000) break;
        if(simhalt) break;
    }
    top->final();
    return tick;
}

int read_int() {
    char buf[0x20] = {0};
    if (read(0, buf, 0x20) < 0) {
        return 0;
    }
    buf[0x1f] = 0;
    return atoi(buf);
}

int read_data(uint8_t *buf, int length) {

    printf("length #> ");
    int recvlen = read_int();
    if (recvlen > length) {
        recvlen = length;
    }

    printf("receiving %d byte(s)\n", recvlen);
    int x = 0;
    while (x < recvlen) {
        int cnt = read(0, &buf[x], recvlen - x);
        if (cnt <= 0) {
            break;
        }
        x += cnt;
    }

    return x;
}

uint32_t menu() {
    printf("\n\
    '##::::::::::'###::::'########:'########:'##:::::::'##:::::::'########:\n\
     ##:::::::::'## ##:::..... ##:: ##.....:: ##::::::: ##::::::: ##.....::\n\
     ##::::::::'##:. ##:::::: ##::: ##::::::: ##::::::: ##::::::: ##:::::::\n\
     ##:::::::'##:::. ##:::: ##:::: ######::: ##::::::: ##::::::: ######:::\n\
     ##::::::: #########::: ##::::: ##...:::: ##::::::: ##::::::: ##...::::\n\
     ##::::::: ##.... ##:: ##:::::: ##::::::: ##::::::: ##::::::: ##:::::::\n\
     ########: ##:::: ##: ########: ########: ########: ########: ########:\n\
    ........::..:::::..::........::........::........::........::........::\n\
\n\
    the future is here - 32bit. but it's too hard to write asm anymore, ya?\n\
    who even wants to write code for some rusty ol' Archimedes....... not I\n\
\n\
    ................ [ WHAT IF THERE WAS AN EASIER WAY ? ] ................\n\
\n\
    1. run example .LEG\n\
    2. try out your own .LEG!\n\
    3. exit\n\
    #> ");

    return read_int();
}

int main(int argc, char *argv[])
{

    setvbuf(stdout, NULL, _IONBF,0);
    setvbuf(stderr, NULL, _IONBF,0);

    memset(flag, 0, sizeof(flag));


    int fd = open("/flag", O_RDONLY);
    if (read(fd, flag, sizeof(flag)) < 0) {
        exit(3);
    }
    close(fd);

    alarm(120);

    int res = 0;
    uint64_t num_ticks = 0;
    uint8_t userdata[0x100] = {0};
    uint32_t *program_buf = NULL;

    while(1) {
        uint32_t option = menu();
        switch(option) {
        case 1: {
            printf("give me your input data\n");
            memset(userdata, 0, sizeof(userdata));
            res = read_data(userdata, 0xfc);
            if (res > 0) {
                // begin run
                sram = (uint32_t *)malloc((SRAMMASK+1));
                ram = (uint32_t *)malloc((RAMMASK+1));
                load_sram();
                map_builtin_program(userdata, res);
                uint64_t num_ticks = simulate();
                printf("simulation finished in %lu tick(s)\n", num_ticks);
                dump_env();

                free(sram);
                free(ram);
            }
            break;
        }
        case 2: {
            printf("give me your .LEG program plz\n");

            program_buf = (uint32_t *)calloc(0x10000, 1);
            res = read_data((uint8_t *)program_buf, 0xfff0);
            if (res > 0) {
                // begin run
                sram = (uint32_t *)malloc((SRAMMASK+1));
                ram = (uint32_t *)malloc((RAMMASK+1));
                load_sram();
                map_lua(program_buf, res);
                uint64_t num_ticks = simulate();
                printf("simulation finished in %lu tick(s)\n", num_ticks);
                dump_env();

                free(sram);
                free(ram);
            }
            free(program_buf);
            break;
        }
        case 3:
            printf("bye\n");
            exit(0);
        default:
            printf("unknown\n");
            break;
        }
    }

    return 0;
}

