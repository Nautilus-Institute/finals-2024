#!/usr/bin/env python

from os import environ
environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'

import os
import math
import pygame
import io
import base64
import sys
import random
import time
import pickle
import subprocess
import json
import gzip
from PIL import Image

import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *


ICON = b'iBL{Q4GJ0x0000DNk~Le0000W0000W2m$~A0Q?y3qW}N_glR)VP)S2WAaHVTW@&6?001bFeUUv#!$2IxUt6Uj6$J~5IAo|US`ZcKs8uLJg-|QB>R@u|7c^-|Qd}Gb*Mfr|i&X~~XI&j!1wrrw#M!|~(M3x9Us7lh<H2!1-ralLy#xI9GE>ct2|(2>BbA5?ne3_%d_@p_7(fhT5;OHVQB1;feBHyx*Sjds@;>+H=uvVe1AGGUEYl5(c!PLm)6zNb6Ngz*Qi#uq#|^q5@gvt|m)|%S9QO0fu#ry96Nia~LL19%%!-CeJWU)?RE_fe8J88#Tb$K$l{N3lUl_`1E6ZG`IgA7ru>=tUWK>Z?85W|nYNVJ*(SF>+KjipDa>?W>fstbY6{wILKlmT~?$*ptO}a^;IMDfG+aDu9a2IIQZ2SAzwi_ov;2F5mTK-BMh<%b?YiW@qpm!U%xNd3k9&ot>3_j_SAvscjrazwt-p}ZpvOxbW(7opN*4)SG1CXY!k~hG?Auw8?>~)WKceVHS@0n(QKi+I|(yqvRCIA2c32;bRa{vGh*8l(w*8xH(n|J^K00(qQO+^Rj2N3`*62H~-0ssI2SV=@dR7l5TV4yKzWME*pvLl8@rt5etVWPDUhQ0+Xn4w0YiD)r88V92mj9M^i0U2cx!Lo(avTHOBsBD3b#}ZO2jL|rtLO!6dmZrN8sNTvOO+^C_0OXPmbgl&J4gdfE07*qoM6N<$f&'

BOARDSIZE = 1000.0
EXTRASIZE = 250.0


verticies_ship = (
    (-10,-10,0),
    (-10,10,0),
    (18,0,0)
    )

edges_ship = (
    (0,1),
    (1,2),
    (2,0)
    )


def Cube():
    glBegin(GL_LINES)
    for edge in edges:
        for vertex in edge:
            glVertex3dv(verticies[vertex])
    glEnd()


def draw_text(value, font=GLUT_BITMAP_8_BY_13, offset=(0,0)):
    glPushMatrix();
    glRasterPos2d(*offset);
    for character in value:
        glutBitmapCharacter(font, ord(character));
    glPopMatrix();


def draw_circle(radius, segments=250, line_width=1.0):
    glLineWidth(line_width);
    glBegin(GL_LINE_LOOP)
    for vertex in range(0, segments):
        angle  = float(vertex) * 2.0 * math.pi / segments
        glVertex3dv((math.cos(angle)*radius, math.sin(angle)*radius,0.0))
    glEnd();
    glLineWidth(1.0);

def draw_score(state, nstates=None):
    glColor3d(0.05, 0.05, 0.10)
    glBegin(GL_POLYGON)
    score_area_vertices = ((BOARDSIZE/2.0+20, BOARDSIZE/2.0),
                        (BOARDSIZE/2.0+EXTRASIZE, BOARDSIZE/2.0), 
                        (BOARDSIZE/2.0+EXTRASIZE, -BOARDSIZE/2.0),
                        (BOARDSIZE/2.0+20, -BOARDSIZE/2.0))
    score_area_edges = ((0,1),(1,2),(2,3),(3,0))
    for edge in score_area_edges:
        for vertex in edge:
            glVertex2dv(score_area_vertices[vertex])
    glEnd()

    glColor3d(1, 1, 1)
    glPushMatrix()
    glTranslated(BOARDSIZE/2.0+30, BOARDSIZE/2.0-40, 0.0)
    draw_text("=== RANKING ===", font=GLUT_BITMAP_HELVETICA_18)
    scored_team = sorted([(team.team_name[:16], team.fscore, team.score) for i, team in state.teams.items()], key=lambda x:-x[1])
    for i, (name, fscore, score) in enumerate(scored_team):
        glTranslated(0, -30, 0.0)
        glPushMatrix()
        draw_text("%s)" % str(i+1))
        glTranslated(22, 0, 0.0)
        draw_text(clean_name(name)) 
        glTranslated(EXTRASIZE-100, 0.0, 0.0)
        draw_text(str(score))
        glPopMatrix()
    glPopMatrix()

    glPushMatrix()
    glTranslated(BOARDSIZE/2.0+30, BOARDSIZE/2.0-40 - 550, 0.0)
    draw_text("=== STATE ===", font=GLUT_BITMAP_HELVETICA_18)
    glTranslated(0, -30, 0.0)
    glPushMatrix()
    if nstates is not None:
        draw_text("tick: %s/%s" % (str(state.tick),str(nstates)))
    else:
        draw_text("tick: %s" % str(state.tick))
    glPopMatrix()
    glTranslated(0, -20, 0.0)
    glPushMatrix()
    draw_text("defcon_round: %s" % str(state.defcon_round))
    glPopMatrix()
    glTranslated(0, -30, 0.0)
    glPushMatrix()
    for e in state.events:
        draw_text(e)
        glTranslated(0, -20, 0.0)
    glPopMatrix()
    glTranslated(0, -60, 0.0)
    for tid, team in state.teams.items():
        if state.moves == None:
            continue
        draw_text("%-3d %-3d: %s" % (tid, team.defcon_id, state.moves[team.tid]))
        glTranslated(0, -15, 0.0)
    glPopMatrix()


def clean_name(name):
    cname = ''.join([i if ord(i) < 128 else 'X' for i in name])
    cname = cname.replace(" ", "")
    return cname


def show(state, images_folder=None, nstates=None):
    for event in pygame.event.get():
        if event.type==pygame.QUIT or (event.type==pygame.KEYDOWN and event.key==ord('q')):
            print("Terminating...")
            pygame.quit()
            sys.exit(0)

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)

    glColor3d(1, 1, 1)
    draw_circle(state.boardradius)
    draw_circle(state.boardradius_inner)

    for i, team in state.teams.items():
        if team.ship == None:
            continue
        l = team.ship.location
        glPushMatrix()
        glTranslated(l.x, l.y, 0.0)
        glRotated(math.degrees(l.a), 0.0, 0.0, 1.0)
        glColor3d(*team.color)
        glBegin(GL_POLYGON)
        for edge in edges_ship:
            for vertex in edge:
                glVertex3dv(verticies_ship[vertex])
        glEnd()
        
        glColor3d(1, 1, 1)
        if(     l.x<(-BOARDSIZE/2.0+20) or l.x>(BOARDSIZE/2.0-20) or
                l.y<(-BOARDSIZE/2.0+20) or l.y>(BOARDSIZE/2.0-20)):
            text_offset = (20,22)
        elif l.a > 0.0 and l.a < math.pi/2.0*1.1:
            text_offset = (-5,-15)
        else:
            text_offset = (-15,10)
        draw_text(clean_name(team.team_name)[:16], offset=text_offset) 
        if team.shield:
            draw_circle(10.0+3.5, segments=25, line_width=3.0)
        glPopMatrix()

    for i, team in state.teams.items():
        if team.ship == None:
            continue
        glColor3d(*team.color)
        for b in team.bullets:
            glPushMatrix()
            glTranslated(b.location.x, b.location.y, 0.0)
            glRotated(random.randint(0, 44), 0.0, 0.0, 1.0)
            glBegin(GL_POLYGON)
            bsize = int(8*2.0)
            glVertex3dv([-bsize,bsize,0])
            glVertex3dv([bsize,bsize,0])
            glVertex3dv([bsize,-bsize,0])
            glVertex3dv([-bsize,-bsize,0])
            glEnd()
            glRotated(45.0, 0.0, 0.0, 1.0)
            glBegin(GL_POLYGON)
            glVertex3dv([-bsize,bsize,0])
            glVertex3dv([bsize,bsize,0])
            glVertex3dv([bsize,-bsize,0])
            glVertex3dv([-bsize,-bsize,0])
            glEnd()
            glPopMatrix()

    draw_score(state, nstates)

    if images_folder is not None:
        data = glReadPixels(0, 0, BOARDSIZE+EXTRASIZE, BOARDSIZE, GL_RGB, GL_UNSIGNED_BYTE, outputType=None)
        image = Image.frombytes("RGB", (int(BOARDSIZE+EXTRASIZE), int(BOARDSIZE)), data)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        image.save(os.path.join(images_folder, "image_%06d.png" % state.tick), format="png")

    pygame.display.flip()


def init():
    pygame.init()
    display = (int(BOARDSIZE + EXTRASIZE),int(BOARDSIZE))
    pygame.display.set_mode(display, DOUBLEBUF|OPENGL)
    pygame.display.set_caption("llmopshipo")
    pygame.display.set_icon(pygame.image.load(io.BytesIO(base64.b85decode(ICON))))
    glutInit()

    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, BOARDSIZE + EXTRASIZE, BOARDSIZE, 0.0, 0.0, 10000.0);
    glMatrixMode(GL_MODELVIEW);
    glTranslated(BOARDSIZE/2.0,BOARDSIZE/2.0, -1.0)
    glRotated(180.0, 1.0, 0.0, 0.0)


class Dummy():
    def __init__(self, dd):
        for k, v in dd.items():
            if k == "teams":
                vnew = {}
                for k2, v2 in v.items():
                    vnew[int(k2)] = Dummy(v2)
                setattr(self, "teams", vnew)
            elif k == "moves" and v!=None:
                vnew = {}
                for k2, v2 in v.items():
                    vnew[int(k2)] = v2
                setattr(self, "moves", vnew)
            elif k == "bullets":
                vnew = []
                for v2 in v:
                    vnew.append(Dummy(v2))
                setattr(self, "bullets", vnew)
            else:
                if type(v) == dict:
                    vnew = Dummy(v)
                else:
                    vnew = v
                setattr(self, k, vnew)


def main(states, images_folder, video=None):
    init()
    with open(states, "rb") as fp:
        states_content = gzip.decompress(fp.read()).decode()

    states = json.loads(states_content)
    for state_dict in states:
        state = Dummy(state_dict)
        print("showing state:", repr(state.tick))
        show(state, images_folder, nstates=len(states)-1)

    if video != None:
        print("rendering video...")
        p = subprocess.Popen(["ffmpeg", "-y", "-framerate", "10", "-i", os.path.join(images_folder, r"image_%06d.png"), "-filter_complex", "pad=width=iw+120:height=ih+120:x=60:y=60:color=black", "-c:v", "libx264", "-crf","15", "-preset", "slow", "-profile:v", "baseline", "-level", "3.0", "-pix_fmt", "yuv420p", "-movflags", "+faststart",  video], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        p.communicate()


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage:\npython3 ./visualizer.py [--video <video_filepath>] <results.gz> <output_image_folder>")
        sys.exit(1)

    try:
        tf = sys.argv.index("--video")
        video = sys.argv[tf+1]
    except ValueError:
        video = None
    main(sys.argv[-2], sys.argv[-1], video)


'''
To run on Ubuntu 18.04:
sudo apt install python3
sudo apt install python3-pip
sudo apt install freeglut3-dev
sudo apt install ffmpeg
sudo pip3 install Pillow pygame PyOpenGL

python3 ./visualizer.py [--video <video_filepath>] <results.gz> <output_image_folder>

To run without a display:
sudo apt install xvfb
Xvfb :1 -screen 0 1024x768x24 < /dev/null &
export DISPLAY=":1"
'''
