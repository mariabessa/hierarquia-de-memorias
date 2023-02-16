#include "mmu.h"
#include <stdio.h>
#include <unistd.h>

bool canOnlyReplaceBlock(Line line) {
    // Or the block is empty or
    // the block is equal to the one in memory RAM and can be replaced
    if (line.tag == INVALID_ADD || (line.tag != INVALID_ADD && !line.updated))
        return true;
    return false;
}

int mapping(Cache* cache, int Map_Type) {

    int posicao = 0;
    
    if(Map_Type == 1) posicao =  cache->size - 1;
    //FIFO
    
    
    else if(Map_Type == 2) {
        //LFU
        int size = cache->size;
        
        for(int i=0; i<size; i++)
            cache->lines[i].uso += 1;
        
        int menor = cache->lines[0].uso;

        for(int i=0; i<size; i++) {
            if(cache->lines[i].estaVazia == 1) {
                cache->lines[i].estaVazia = 0;
                return i;
            }
            if(menor > cache->lines[i].uso) {
                menor = cache->lines[i].uso;
                posicao = i;
            }
        }
    }
    
    return posicao;
}

int memoryCacheMapping(Line* cache, int tam, int block, int* lpos) {
    for(int i = 0; i < tam; i++)
        if(cache[i].tag == block) {
            *lpos = i;
            return 1;
        }
    return 0;
}

void FIFO(Cache* cache) {
    for(int i = cache->size - 1; i > 0; i--)
        cache->lines[i] = cache->lines[i - 1];
}



void updateMachineInfos(Machine* machine, Line* line) {
    switch (line->cacheHit) {
        case 1:
            machine->hitL1 += 1;
            break;

        case 2:
            machine->hitL2 += 1;
            machine->missL1 += 1;
            break;
            
        case 3:
            machine->hitL3 += 1;
            machine->missL1 += 1;
            machine->missL2 += 1;
            break;
        
        case 4:
            machine->hitRAM += 1;
            machine->missL1 += 1;
            machine->missL2 += 1;
            machine->missL3 += 1;
            break;
    }
    machine->totalCost += line->cost;
}

Line* MMUSearchOnMemorys(Address add, Machine* machine, int Map_Type) {
    // Strategy => write back
    
    // Direct memory map
    // int l1pos = memoryCacheMapping(&machine->l1, Map_Type);
    // int l3pos = memoryCacheMapping(&machine->l3, Map_Type);
    // int l2pos = memoryCacheMapping(&machine->l2, Map_Type);
    int l1pos;
    int l2pos;
    int l3pos;

    Line* cache1 = machine->l1.lines;
    Line* cache2 = machine->l2.lines;
    Line* cache3 = machine->l3.lines;
    MemoryBlock* RAM = machine->ram.blocks;

    if (memoryCacheMapping(cache1, machine->l1.size, add.block, &l1pos)) {
        /* Block is in memory cache L1 */
        // cache3[l1pos].tag = add.block;
        cache1[l1pos].cost = COST_ACCESS_L1;
        cache1[l1pos].cacheHit = 1;
        cache1[l1pos].uso += 1;
        
        sleep(0.001);
   
    } else if (memoryCacheMapping(cache2, machine->l2.size, add.block, &l2pos)) {
        /* Block is in memory cache L2 */
        cache2[l2pos].tag = add.block;
        cache2[l2pos].updated = false;
        cache2[l2pos].cost = COST_ACCESS_L1 + COST_ACCESS_L2;
        cache2[l2pos].cacheHit = 2;
        cache2[l2pos].uso += 1;

        // !Can be improved?
        sleep(0.011);

        updateMachineInfos(machine, &(cache2[l2pos]));
        return &(cache2[l2pos]);
    // } else if (cache3[l3pos].tag == add.block) { 
    } else if (memoryCacheMapping(cache3, machine->l3.size, add.block, &l3pos)) {
        /* Block is in memory cache L3 */
        cache3[l3pos].tag = add.block;
        cache2[l2pos].updated = false;
        cache3[l3pos].cost = COST_ACCESS_L1 + COST_ACCESS_L2 + COST_ACCESS_L3;
        cache3[l3pos].cacheHit = 3;
        cache3[l3pos].uso += 1;
        
        // !Can be improved?
        sleep(0.111);
        
        updateMachineInfos(machine, &(cache3[l3pos]));
        return &(cache3[l3pos]);
    } else { 
        /* Block only in memory RAM, need to bring it to cache and manipulate the blocks */
        l1pos = mapping(&machine->l1, Map_Type); /* Need to check the position of the block that will leave the L1 */
        l2pos = mapping(&machine->l2, Map_Type); /* Need to check the position of the block that will leave the L1 */
        l3pos = mapping(&machine->l3, Map_Type); /* Need to check the position of the block that will leave the L2 */
        
        if (!canOnlyReplaceBlock(cache1[l1pos])) { 
            /* The block on cache L1 cannot only be replaced, the memories must be updated */
            if (!canOnlyReplaceBlock(cache2[l2pos])){
                /* The block on cache L2 cannot only be replaced, the memories must be updated */
                if(!canOnlyReplaceBlock(cache3[l3pos])) {
                    /* The block on cache L3 cannot only be replaced, the memories must be updated */
                
                    RAM[cache3[l3pos].tag] = cache3[l3pos].block;
                }
                if(Map_Type == 1) {
                    FIFO(&machine->l3);
                    cache3[0] = cache2[l2pos];
                } else
                    cache3[l3pos] = cache2[l2pos];

            }
            if(Map_Type == 1) {
                FIFO(&machine->l2);
                cache2[0] = cache1[l1pos];
            } else
                cache2[l2pos] = cache1[l1pos];
        }
        if(Map_Type == 1) {
            FIFO(&machine->l1);
            l1pos = 0;
        }
        cache1[l1pos].block = RAM[add.block];
        cache1[l1pos].tag = add.block;
        cache1[l1pos].updated = false;
        cache1[l1pos].cost = COST_ACCESS_L1 + COST_ACCESS_L2 + COST_ACCESS_L3 + COST_ACCESS_RAM;
        cache1[l1pos].cacheHit = 4;
        cache1[l1pos].uso = 0;
        sleep(1.111);
    }
    updateMachineInfos(machine, &(cache1[l1pos]));
    return &(cache1[l1pos]);
}
