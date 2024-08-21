#!/usr/bin/env python


import os
import math
import time
import copy
import random
import sys
from collections import OrderedDict


from state import *

BOARDSIZE = 1000.0
BOARDRADIUS = 0.4991
BOARDRADIUS_INNER = 0.125/2.0
TEAMRADIUS = 0.35
BULLETDELAY = int(50/2)
SHIELDDELAY = int(150/2)
SHIELDTIME = int(50/2)
HITBOX = int(20*2.0)
DEATHSCORE = -1000
KILLSCORE = 400
MAXTICK = 100000
GAMENTICK = 1000
MAXFSPEED = 9.0*2.0
MAXASPEED = 0.20*2.0
FSPEEDINCREMENT = 1.0*2.0
ASPEEDINCREMENT = 0.03*2.0

assert(GAMENTICK < MAXTICK-11)

def next_state(state, moves):

    state.moves = moves

    state.tick += 1

    state.events = []
    
    for tid, team in state.teams.items():
        if team.ship == None:
            continue
        if team.shield == True:
            if state.tick - team.last_shield >= SHIELDTIME:
                team.shield = False

    for tid, move in moves.items():
        team = state.teams[tid]
        if team.ship == None:
            continue
        if move == b"a":
            if state.tick - team.last_shooting >= BULLETDELAY:
                team.last_shooting = state.tick
                b = Bullet(copy.deepcopy(team.ship.location));
                b.location.move(t=b.fspeed*2)
                team.bullets.append(b)
        elif move == b"s":
            if state.tick - team.last_shield >= SHIELDDELAY:
                team.last_shield = state.tick
                team.shield = True

    to_kill_teams = set()

    for tid, move in moves.items():
        team = state.teams[tid]
        if team.ship == None:
            continue

        if move == b"n":
            pass
        elif move == b"e":
            to_kill_teams.add((tid,tid))
        elif move == b"u":
            team.ship.fspeed += FSPEEDINCREMENT
            if team.ship.fspeed > MAXFSPEED:
                team.ship.fspeed = MAXFSPEED
        elif move == b"d":
            team.ship.fspeed -= FSPEEDINCREMENT
            if team.ship.fspeed < -5.0:
                team.ship.fspeed = -5.0
        elif move == b"l":
            team.ship.aspeed += ASPEEDINCREMENT
            if team.ship.aspeed > MAXASPEED:
                team.ship.aspeed = MAXASPEED
        elif move == b"r":
            team.ship.aspeed -= ASPEEDINCREMENT
            if team.ship.aspeed < -MAXASPEED:
                team.ship.aspeed = -MAXASPEED

        if move != b"u" and move != b"d":
            team.ship.fspeed = 0 ### was 0.8
            if math.fabs(team.ship.fspeed) < 0.25:
                team.ship.fspeed = 0.0

        if move != b"l" and move != b"r":
            team.ship.aspeed = 0 ### was 0.25
            if math.fabs(team.ship.aspeed) < 0.005:
                team.ship.aspeed = 0.0

    for i, team in state.teams.items():
        if team.ship == None:
            continue
        team.ship.location.move(a=team.ship.aspeed)
        if team.ship.fspeed != 0.0:
            new_location = copy.deepcopy(team.ship.location)
            new_location.move(t=team.ship.fspeed)
            new_location_distance = new_location.distance(Location(0, 0))

            if new_location_distance >= (BOARDSIZE*BOARDRADIUS):
                old_location_distance = team.ship.location.distance(Location(0, 0))
                segment_length = new_location_distance - old_location_distance
                if segment_length != 0.0:
                    ratio_outside = (new_location_distance - (BOARDSIZE*BOARDRADIUS)) / segment_length
                    team.ship.location.move(t=team.ship.fspeed*(1.0-ratio_outside))
                else:
                    pass
                team.ship.fspeed = 0.0

            elif new_location_distance <= (BOARDSIZE*BOARDRADIUS_INNER):
                old_location_distance = team.ship.location.distance(Location(0, 0))
                segment_length =  old_location_distance - new_location_distance

                if segment_length != 0.0:
                    ratio_outside = ((BOARDSIZE*BOARDRADIUS_INNER) - new_location_distance) / segment_length
                    team.ship.location.move(t=team.ship.fspeed*(max((1.0-(ratio_outside)),0.0)))
                else:
                    pass
                team.ship.fspeed = 0.0

            else:
                team.ship.location = new_location

    to_remove_bullets = set()
    for i, team in state.teams.items():
        if team.ship == None:
            continue
        for b in team.bullets:
            new_location = copy.deepcopy(b.location)
            new_location.move(t=b.fspeed) 
            for j, otherteam in state.teams.items():
                if otherteam.ship == None:
                    continue
                if otherteam.tid == team.tid:
                    continue
                if b.location.distance3(new_location, otherteam.ship.location) <= HITBOX:
                    if otherteam.shield == False:
                        to_kill_teams.add((j, i))
                    to_remove_bullets.add((i, b))
            b.location = new_location

    for team_id, killer_id in to_kill_teams:
        t = state.teams[team_id]
        t.ship = None
        t.bullets = []
        if team_id == killer_id:
            t.score += DEATHSCORE
        t.score += DEATHSCORE
        t.died_tick = state.tick
        state.teams[killer_id].score += KILLSCORE
        state.teams[killer_id].lastkill_tick = state.tick
        state.events.append("%s killed %s" % (state.teams[killer_id].team_name[:10], state.teams[team_id].team_name[:10]))
    for team_id, bullet in to_remove_bullets:
        try:
            state.teams[team_id].bullets.remove(bullet)
        except ValueError:
            pass



    for i, team in state.teams.items():
        team.bullets = [t for t in team.bullets if t.location.distance(Location()) < (BOARDSIZE*BOARDRADIUS) and t.location.distance(Location()) > (BOARDSIZE*BOARDRADIUS_INNER)]

    for tid, t in state.teams.items():
        if t.lastkill_tick == 0:
            lastkill_tick  = MAXTICK
        else:
            lastkill_tick = t.lastkill_tick
        if t.died_tick == 0:
            died_tick = state.tick
        else:
            died_tick = t.died_tick
        t.fscore = t.score + died_tick / float(MAXTICK) + (MAXTICK - lastkill_tick) / float(10*MAXTICK*MAXTICK)

    scoringteam_list = [t for t in state.teams.values() if t.fscore > 0.999]
    for team in state.teams.values():
        if team not in scoringteam_list:
            team.rank = 0
    scoringteam_list.sort(key=lambda x: x.fscore)
    for i, team in enumerate(scoringteam_list):
        team.rank = i+1


    return state





def init(teams_with_ais, defcon_round, seed, gamentick, defcon_ids_list=None):
    state = State(teams_with_ais, BOARDRADIUS*BOARDSIZE, BOARDRADIUS_INNER*BOARDSIZE, -BOARDSIZE*TEAMRADIUS, defcon_round, seed, True, gamentick, defcon_ids_list)
    return state

