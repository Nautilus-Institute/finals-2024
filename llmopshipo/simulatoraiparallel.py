#!/usr/bin/env python

from os import environ
environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'

import sys
import time
from datetime import datetime
from ctypes import cdll
import copy
import gzip
import json
import codecs
import os
import shutil
import pickle
import random
import math

import LLM
import game
import stateconverter

DEFCONROUND = None
TEAMS = None
DEBUGLLM = True

BASEPATH = "/shared"
'''
The uploaded prompts will be stored in PROMPTS_PATH/<your_team_name>.txt
'''
PROMPTS_PATH = BASEPATH+"/prompts/"
NROUNDS = 500

PROMPT_MAX_LEN = 5000*2



def previous_moves_to_str(previous_moves, defcon_id, state):
    tstrl = []
    tstrl.append("\n\nThese are the moves performed by each of the other teams in the previous tick")
    for k,v in previous_moves.items():
        if k == defcon_id:
            continue
        tid = state.teams_by_defcon_id[k].tid
        tstrl.append("Team "+str(tid)+" move was: "+previous_moves[k])

    tstr = "\n".join(tstrl)+"\n\n"
    return tstr


def tag_translator(state, defcon_id, tag):
    #tags = ["teamid", "teamname", "tick", "speed", "aspeed", "shields", "shielda", "weapona", "ocirclef", "ocircler", "ocirclel", "ocircleb", "icirclef", "icircler". "icirclel", "icircleb", "others1d", "others1a", "others1s", "others2d", "others2a", "others2s", "others3d", "others3a", "others3s", "otherb1d", "otherb1a", "otherb2d", "otherb2a", "otherb3d", "otherb3a", "otherb1x", "otherb2x", "otherb3x"]
    
    sd = stateconverter.state_to_nnstate(state, defcon_id)

    if tag == "teamid":
        return str(state.teams_by_defcon_id[defcon_id].tid)
    elif tag == "teamname":
        return str(state.teams_by_defcon_id[defcon_id].team_name)
    elif tag == "tick":
        return str(state.tick)
    elif tag == "speed":
        return str(round(sd["fspeed"],2))
    elif tag == "aspeed":
        return str(round(math.fabs(sd["aspeed"]),2)) + (" leftward" if sd["aspeed"] <0.0 else " rightward")
    elif tag == "shields":
        return ("active" if sd["shield"] else "not active")
    elif tag == "shielda":
        return ("not available" if sd["shield_available"] < 1.0 else "available")
    elif tag == "weapona":
        return ("not available" if sd["attack_available"] < 1.0 else "available")
    elif tag == "ocirclef":
        return str(int(sd["ocirclef"]))
    elif tag == "ocircler":
        return str(int(sd["ocircler"]))
    elif tag == "ocirclel":
        return str(int(sd["ocirclel"]))
    elif tag == "ocircleb":
        return str(int(sd["ocircleb"]))
    elif tag == "icirclef":
        if sd["icirclef"] != float('inf'):
            return str(int(sd["icirclef"]))
        else:
            return "infinite"
    elif tag == "icircler":
        if sd["icircler"] != float('inf'):
            return str(int(sd["icircler"]))
        else:
            return "infinite"
    elif tag == "icirclel":
        if sd["icirclel"] != float('inf'):
            return str(int(sd["icirclel"]))
        else:
            return "infinite"
    elif tag == "icircleb":
        if sd["icircleb"] != float('inf'):
            return str(int(sd["icircleb"]))
        else:
            return "infinite"

    elif tag in ["others1d", "others2d", "others3d"]:
        idx = int(tag.replace("others","").replace("d",""))-1
        if idx<len(sd["otherships"]):
            s = sd["otherships"][idx]
            return str(int(s[0]))
        else:
            return "None"

    elif tag in ["others1a", "others2a", "others3a"]:
        idx = int(tag.replace("others","").replace("a",""))-1
        if idx<len(sd["otherships"]):
            s = sd["otherships"][idx]
            return str(int(s[1]/3.1415*180))
        else:
            return "None"

    elif tag in ["others1s", "others2s", "others3s"]:
        idx = int(tag.replace("others","").replace("s",""))-1
        if idx<len(sd["otherships"]):
            s = sd["otherships"][idx]
            return ("activated" if s[2] else "not activated")
        else:
            return "None"

    elif tag in ["otherb1d", "otherb2d", "otherb3d"]:
        idx = int(tag.replace("otherb","").replace("d",""))-1
        if idx<len(sd["otherbullets"]):
            s = sd["otherbullets"][idx]
            return str(int(s[0]))
        else:
            return "None"

    elif tag in ["otherb1a", "otherb2a", "otherb3a"]:
        idx = int(tag.replace("otherb","").replace("a",""))-1
        if idx<len(sd["otherbullets"]):
            s = sd["otherbullets"][idx]
            return str(int(s[1]/3.1415*180))
        else:
            return "None"

    elif tag in ["otherb1x", "otherb2x", "otherb3x"]:
        idx = int(tag.replace("otherb","").replace("x",""))-1
        if idx<len(sd["otherbullets"]):
            s = sd["otherbullets"][idx]
            return str(int(s[2]/3.1415*180))
        else:
            return "None"

    return "ERROR"


def dump_states(states):
    def serialize(obj):
        if type(obj) == bytes:
            return codecs.decode(obj)
        return obj.__dict__

    print("="*3, datetime.now(), "dumping states", "tick", states[-1].tick)
    gzip.compress(json.dumps(states, default=serialize,separators=(',', ':')).encode("utf-8"))
    with open(os.path.join(BASEPATH, "states_tmp"), "wb") as fp:
        fp.write(gzip.compress(json.dumps(states, default=serialize,separators=(',', ':')).encode("utf-8")))
    shutil.move(os.path.join(BASEPATH, "states_tmp"), os.path.join(BASEPATH, "states"))


def run(teams_with_prompts, gamentick, generation=0, visualize=False, defcon_ids_list=None, save_states=True):
    if visualize:
        import visualizer
        visualizer.init()

    defconround = generation

    states = []
    state = game.init(teams_with_prompts, defconround, 1, gamentick, defcon_ids_list=defcon_ids_list)

    states.append(copy.deepcopy(state))

    previous_moves = {}
    for k,v in state.teams_by_defcon_id.items():
        previous_moves[k] = ""
    while True:
        if state.tick%100 == 0: print("=", state.tick)

        if state.tick%10==0:
            dump_states(states)

        ALTMODEL = False
        if state.tick > 5:
            if (state.tick % 10) == 0 or (state.tick % 10) == 1:
                ALTMODEL = True
        LLM.ALTMODEL = ALTMODEL

        if visualize:
            visualizer.show(state, None, gamentick)

        t1 = time.time()
        moves = {}

        full_prompts={}
        for defcon_id, v in state.teams_by_defcon_id.items():
            system_prompt, prompt1, previous_moves_prompt, prompt2 = "", "", "", ""
            if state.teams_by_defcon_id[defcon_id].ship != None:
                previous_moves_prompt = previous_moves_to_str(previous_moves, defcon_id, state)
                splitter = "\n"+"="*10+"\n"
                #print(teams_with_prompts[defcon_id][1])
                splitted_prompt = teams_with_prompts[defcon_id][1].split(splitter)
                #print(splitted_prompt)
                if len(splitted_prompt) == 6:
                    if not ALTMODEL:
                        system_prompt, prompt1, prompt2 = splitted_prompt[:3]
                    else:
                        system_prompt, prompt1, prompt2 = splitted_prompt[3:]

                    tags = ["teamid", "teamname", "tick", "speed", "aspeed", "shields", "shielda", "weapona", "ocirclef", "ocircler", "ocirclel", "ocircleb", "icirclef", "icircler", "icirclel", "icircleb", "others1d", "others1a", "others1s", "others2d", "others2a", "others2s", "others3d", "others3a", "others3s", "otherb1d", "otherb1a", "otherb2d", "otherb2a", "otherb3d", "otherb3a", "otherb1x", "otherb2x", "otherb3x"]
                    for t in tags:
                        ft = "[["+t+"]]"
                        if ft in prompt1:
                            prompt1 = prompt1.replace(ft,tag_translator(state,defcon_id,t),1)

            system_prompt = system_prompt[:500]
            system_prompt += "\n\nAlways answer starting with one of the following words:\nup, down, left, right, attack, shield, leave"
            prompt1 = prompt1[:750]
            prompt2 = prompt2[:500]

            full_prompts[defcon_id] = (system_prompt,prompt1+previous_moves_prompt+prompt2)

        
        answer_move_dict, previous_moves = LLM.parallel_execute_prompts(full_prompts)

        moves = {state.teams_by_defcon_id[k].tid: v for k,v in answer_move_dict.items()}
        state = game.next_state(state, moves)
        state.answers = previous_moves

        if save_states:
            states.append(copy.deepcopy(state))

        if state.tick >= gamentick-1:
            return states


def decode_dict(tstr):
    d = {}
    for kv in tstr.split(b"_"):
        k, v = kv.split(b"-")
        d[int(k)] = codecs.decode(codecs.decode(v, "hex"))
    return d


def encode_dict(d):
    assert all((type(k)==int and type(v)==str for k, v in d.items()))
    return b"_".join((codecs.encode(str(k))+b"-"+codecs.encode(codecs.encode(v), "hex") for k, v in d.items()))



def main(defconround, teams, nrounds):
    teams_with_prompts = {}
    for i, t in teams.items():
        fname = os.path.join(PROMPTS_PATH,t+".txt")
        try:
            with open(fname, "rb") as fd:
                prompt = fd.read(PROMPT_MAX_LEN)
                prompt = prompt.decode('utf-8', 'ignore')
        except OSError: # if the prompt file is not there, we return an empty prompt
            prompt = ""
        teams_with_prompts[i] = (t, prompt)

    states = run(teams_with_prompts, nrounds, generation=defconround, visualize=False) 
    dump_states(states)


if __name__ == "__main__":
    cdll['libc.so.6'].prctl(1, 9)

    defconround = int(sys.argv[1])
    teams = decode_dict(sys.argv[2].encode(sys.getfilesystemencoding(), 'surrogateescape'))
    if len(sys.argv)>=4:
        nrounds = int(sys.argv[3])
    else:
        nrounds = NROUNDS

    print("="*10, "START", defconround, datetime.now())
    print(nrounds)
    print(repr(teams))
    DEFCONROUND = defconround
    TEAMS = teams

    LLM.DEBUGLLM = DEBUGLLM
    LLM.DEFCONROUND = DEFCONROUND
    LLM.TEAMS = TEAMS
    if(DEBUGLLM):
        os.system("mkdir "+BASEPATH+"/"+"debugllm/")
        os.system("mkdir "+BASEPATH+"/"+"debugllm/"+str(DEFCONROUND))
        LLM.DEBUGPATH = BASEPATH+"/debugllm/"+str(DEFCONROUND)

    main(defconround, teams, nrounds)
  
    print("="*10, "END", defconround, datetime.now())


