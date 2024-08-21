
import random
import re
import time
import os
from groq import Groq
import aiohttp
import asyncio
import requests
import openai


# This variable defines the max number of **Output** tokens for each answer
MAX_OUTPUT_TOKENS = 50



#DEEPINFRA_API_KEY = open("apikey.secret").read().strip()
#DEEPINFRA_API_URL = "https://api.deepinfra.com/v1/inference/meta-llama/Llama-2-7b-chat-hf"

api_key = open("anyscale.secret").read().strip()


def query_llm_sync(prompt_tuple, ids, max_tokens=MAX_OUTPUT_TOKENS):
    system_prompt, fp = prompt_tuple
    if not fp:
        res = "None", ids

    else:
    
    #client = openai.OpenAI(
    #    base_url = "https://api.endpoints.anyscale.com/v1",
    #    api_key = anyscale_api_key
    #)

        client = openai.OpenAI(
        base_url = "https://api.novita.ai/v3/openai",
        api_key = api_key
        )

        # Note: Anyscale doesn't support all arguments and ignores some of them in the backend.
        #otime = time.time()

        if not ALTMODEL:
            model = "meta-llama/llama-3.1-8b-instruct"
        else:
            model = "meta-llama/llama-3.1-405b-instruct"

        try:
            response = client.chat.completions.create(
            model=model,
                max_tokens=max_tokens,
                messages=[{"role": "system", "content": system_prompt},
                        {"role": "user", "content": fp}],
                temperature=0.1
            )
        #print(time.time()-otime)
            res = response.choices[0].message.content, ids
        except openai.InternalServerError:
            print("OPENAI ERROR!")
            res = "None", ids

    ###START_REMOVE
    if DEBUGLLM:
        with open(DEBUGPATH+"/"+TEAMS[ids],"a") as f:
            f.write("\n"+"="*30)
            f.write("\n----------------- SYSTEM PROMPT FOR "+repr(ids)+" IS\n")
            f.write(system_prompt)
            f.write("\n----------------- NORMAL PROMPT IS\n")
            f.write(fp)
            f.write("\n----------------- OUTPUT IS\n")
            f.write(res[0])
            f.write("\n-----------------\n")


    ###END_REMOVE


    return res

async def query_llm(prompt_tuple, ids):
    #print("INPUT STRING",str(input_string))
    return await asyncio.to_thread(query_llm_sync, prompt_tuple, ids)

async def llm_parallel(prompt_tuples, ids_list):
    tasks = [query_llm(prompt_tuple, ids) for (prompt_tuple, ids) in zip(prompt_tuples, ids_list)]
    return await asyncio.gather(*tasks)



def parallel_execute_prompts(prompts):
    ids_list = []
    prompt_list = []
    for defcon_id, (system_prompt,fp) in prompts.items():
        ids_list.append(defcon_id)
        prompt_list.append((system_prompt,fp))

    timer = time.time()
    answer_list=  asyncio.run(llm_parallel(prompt_list, ids_list))
    #for i,a in enumerate(answer_list):
        #print("====================\nANSWER", i)
        #print(a)
    print("Time taken for LLM:", time.time()-timer)

    possible_moves = [b'n', b'u', b'd', b'l', b'r', b'a', b's',b'e']
    move_str_list = ["none","up","down","left","right","attack","shield","leave"]

    answer_move_list = []
    for index in range(len(answer_list)):
        a = answer_list[index][0]
        for i,ms in enumerate(move_str_list):
            if a.lower().startswith(ms):
                answer_move_list.append(possible_moves[i])
                break
        else:
            answer_move_list.append(b"")
            answer_list[index] = ("invalid",answer_list[index][1])

    answer_move_dict = {}
    answer_dict = {}
    for i,defcon_id in enumerate(ids_list):
        #print("DEFCON ID", defcon_id)
        answer_move_dict[defcon_id] = answer_move_list[i]
        answer_dict[defcon_id] = answer_list[i][0]

    return answer_move_dict, answer_dict

