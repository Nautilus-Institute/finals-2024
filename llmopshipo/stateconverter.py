
import game

import copy
import math
from collections import OrderedDict
import shapely
from shapely.geometry import Point
from shapely.geometry import LineString
import struct
import os


def state_to_nnstate(state, defcon_id):
    def ls(v, mx, mn = 0):
        return round((float(v)-float(mn)) / float(mx-mn),8)
    
    def bs(v):
        return 1.0 if v else 0.0

    def distance(x1,y1,x2,y2):
        dx = x1-x2
        dy = y1-y2
        return math.sqrt(dx**2+dy**2)

    def coord_to_distance(sx, sy, ix, iy, radius):
            d = distance(sx, sy, ix, iy)
            return d

    def location_to_outer_circle(location, radius, direction=0):
        l2 = copy.deepcopy(location)
        l2.move(a=direction,t=10000.0)
        p = Point(0,0)
        c = p.buffer(radius*1.002).boundary
        l = LineString([(location.x, location.y), (l2.x, l2.y)])
        i = c.intersection(l)

        assert i.geom_type == "Point"
        if not i.is_empty:
            return coord_to_distance(location.x, location.y, i.xy[0][0], i.xy[1][0], radius)
        else:
            return 1.0

    def location_to_inner_circle(location, radius, direction=0):
        l2 = copy.deepcopy(location)
        l2.move(a=direction,t=10000.0)
        p = Point(0,0)
        c = p.buffer(radius*0.998).boundary
        l = LineString([(location.x, location.y), (l2.x, l2.y)])
        i = c.intersection(l)

        #import IPython; IPython.embed()
        #print(l,i, "i")

        if i.is_empty:
            #print(i)
            return float('inf')

        #import IPython; IPython.embed()
        if i.geom_type == "Point":
            return coord_to_distance(location.x, location.y, i.xy[0][0], i.xy[1][0], radius)
        else:
            assert i.geom_type == "MultiPoint"
            assert len(i.geoms) == 2
            i1, i2 = list(i.geoms)
            sd1 = coord_to_distance(location.x, location.y, i1.xy[0][0], i1.xy[1][0], radius)
            sd2 = coord_to_distance(location.x, location.y, i2.xy[0][0], i2.xy[1][0], radius)
            return max(sd1, sd2)


    def team_to_dict(state, team):
        tstate = OrderedDict()
        #tstate["xxx"] = 1.0 
        tstate["fspeed"] = team.ship.fspeed
        tstate["aspeed"] = team.ship.aspeed
        tstate["shield"] = team.shield
        tstate["shield_available"] = min((state.tick - team.last_shield)/float(game.SHIELDDELAY), 1.0)
        tstate["attack_available"] = min((state.tick - team.last_shooting)/float(game.BULLETDELAY), 1.0)

        tstate["ocirclef"] = location_to_outer_circle(team.ship.location, state.boardradius, direction=0)
        tstate["ocircleb"] = location_to_outer_circle(team.ship.location, state.boardradius, direction=math.pi)
        tstate["ocirclel"] = location_to_outer_circle(team.ship.location, state.boardradius, direction=math.pi/2.0)
        tstate["ocircler"] = location_to_outer_circle(team.ship.location, state.boardradius, direction=-math.pi/2.0)
        tstate["icirclef"] = location_to_inner_circle(team.ship.location, state.boardradius_inner, direction=0)
        tstate["icircleb"] = location_to_inner_circle(team.ship.location, state.boardradius_inner, direction=math.pi)
        tstate["icirclel"] = location_to_inner_circle(team.ship.location, state.boardradius_inner, direction=math.pi/2.0)
        tstate["icircler"] = location_to_inner_circle(team.ship.location, state.boardradius_inner, direction=-math.pi/2.0)

        slist_distances = []
        for tid, team2 in state.teams.items():
            if team2 == team:
                continue
            if team2.ship == None:
                continue
            else:
                dd = distance(team2.ship.location.x, team2.ship.location.y, team.ship.location.x, team.ship.location.y)
                rda = math.atan2(team2.ship.location.y-team.ship.location.y, team2.ship.location.x-team.ship.location.x)
                rda = rda - team.ship.location.a
                if rda>math.pi:
                    rda -= 2*math.pi
                elif rda<-1.0*math.pi:
                    rda += 2*math.pi
                slist_distances.append((dd, rda, team2.shield))

        sorted_distances = sorted(slist_distances, key=lambda x:round(x[0],10)+x[1]/100000000000)
        tstate["otherships"] = sorted_distances[:3]

        slist_distances = []
        for tid, team2 in state.teams.items():
            if team2 == team:
                continue
            for b in team2.bullets:
                dd = distance(b.location.x, b.location.y, team.ship.location.x, team.ship.location.y)
                rda = math.atan2(b.location.y-team.ship.location.y, b.location.x-team.ship.location.x)
                rda = rda - team.ship.location.a
                if rda>math.pi:
                    rda -= 2*math.pi
                elif rda<-1.0*math.pi:
                    rda += 2*math.pi
                rdb = b.location.a - team.ship.location.a
                if rdb>math.pi:
                    rdb -= 2*math.pi
                elif rdb<-1.0*math.pi:
                    rdb += 2*math.pi
                slist_distances.append((dd,rda,rdb))
        sorted_distances = sorted(slist_distances, key=lambda x:round(x[0],10)+x[1]/100000000000)
        tstate["otherbullets"] = sorted_distances[:3]

        return tstate



    team = state.teams_by_defcon_id[defcon_id]
    return team_to_dict(state, team)


def serialize(nnstate, fname):
    try:
        os.unlink(fname)
    except OSError:
        pass
    vl = list(nnstate.values())
    with open(fname, "wb") as fp:
        fp.write(struct.pack("="+str(len(vl))+"f", *vl))

