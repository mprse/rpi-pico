import pygame
import time
import random
import os
import socket
import multiprocessing
from time import sleep

white = (255, 255, 255)
yellow = (255, 255, 102)
black = (0, 0, 0)
red = (213, 50, 80)
green = (0, 255, 0)
blue = (50, 153, 213)

class Snake:
    def __init__(self):
        self.snake = []
        self.score = 0
        self.score_offset = 30
        self.dis_width = 600
        self.dis_height = 400 + self.score_offset
        self.pos_x = 30
        self.pos_y = 10 + self.score_offset
        self.move_x = 10
        self.move_y = 0
        self.food_x = 0
        self.food_y = 0
        self.snake_block = 10
        self.food_block = 10
        self.score = 0
        self.hi_score = 0
        
        pygame.init()
        
        if os.path.exists("score.txt"):
            file_score = open('score.txt', 'r')
            self.hi_score = int(file_score.readline())

        self.status_font = pygame.font.SysFont("bahnschrift", 50)
        self.score_font = pygame.font.SysFont("comicsansms", 20)
        
        self.display = pygame.display.set_mode((self.dis_width, self.dis_height))
        pygame.display.set_caption('Wąż Bibiego i Poli')
        
        self.clock = pygame.time.Clock()

    def reset_snake(self):
        self.pos_x = 10
        self.pos_y = 10 + self.score_offset
        self.move_x = 10
        self.move_y = 0
        self.snake = [pygame.Rect(self.pos_x, self.pos_y, self.snake_block, self.snake_block), 
            pygame.Rect(self.pos_x + self.snake_block, self.pos_y, self.snake_block, self.snake_block), 
            pygame.Rect(self.pos_x + 2 * self.snake_block, self.pos_y, self.snake_block, self.snake_block)]
        self.pos_x = 30
        self.score = 0
        self.draw_food()
        
    def is_game_over(self):
        # check boundary condition
        if (self.pos_x < 0 or self.pos_x > (self.dis_width - self.snake_block)) or (self.pos_y < self.score_offset or self.pos_y > (self.dis_height - self.snake_block)):
            return True
        # check if we hit ourself
        for rect in self.snake:
            if (rect.x == self.pos_x and rect.y == self.pos_y):
                return True
        return False

    def draw_food(self):
        food_pos_ok = False
        while (not food_pos_ok):
            self.food_x = random.randrange(0, int(self.dis_width / 10)) * 10
            self.food_y = random.randrange(0, int((self.dis_height - self.score_offset) / 10)) * 10 + self.score_offset
            
            #Redraw if food is on snake
            for rect in self.snake:
                if (rect.x == self.food_x and rect.y == self.food_y):
                    continue
            break
        
    def game_loop(self):
        game_over = False
        game_on = True
        game_pause = False
        self.reset_snake()

        while game_on:
            # handle events from keyboard when game is over or paused
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    print("quit 1")
                    game_on = False
                    return
                if event.type == pygame.KEYDOWN:
                    if game_over:
                        game_over = False
                        self.reset_snake()
                    if event.key == pygame.K_SPACE:
                        game_pause = False
                        pygame.event.clear()

            while (not game_over) and (not game_pause):
                self.display.fill(blue)
                # black background for score
                score_rect = pygame.Rect(0, 0, self.dis_width, self.score_offset)
                pygame.draw.rect(self.display, black, score_rect)
                # print score
                score_surface = self.score_font.render("SCORE: " + str(self.score), False, red)
                self.display.blit(score_surface, (10, 0))
                hi_score_surface = self.score_font.render("HIGH SCORE " + str(self.hi_score), False, red)
                self.display.blit(hi_score_surface, (400, 0))
                
                # handle events from keyboard during game
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        game_over = True
                        game_on = False
                        return
                    if event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_LEFT:
                            if self.move_x != 10:
                                self.move_x = -10
                                self.move_y = 0
                            pass
                        elif event.key == pygame.K_RIGHT:
                            if self.move_x != -10:
                                self.move_x = 10
                                self.move_y = 0
                            pass
                        elif event.key == pygame.K_UP:
                            if self.move_y != 10:
                                self.move_y = -10
                                self.move_x = 0
                            pass
                        elif event.key == pygame.K_DOWN:
                            if self.move_y != -10:
                                self.move_y = 10
                                self.move_x = 0
                            pass
                        elif event.key == pygame.K_SPACE:
                            game_pause = True
                            text_surface = self.status_font.render('Game Paused', False, red)
                            self.display.blit(text_surface, (self.dis_width / 2 - 120, self.dis_height / 2 - 30))
                            pass
                
                # calculate new snake position
                self.pos_x += self.move_x
                self.pos_y += self.move_y
                
                if self.is_game_over() or game_over is True:
                    game_over = True
                    text_surface = self.status_font.render('Game Over', False, red)
                    self.display.blit(text_surface, (self.dis_width / 2 - 100, self.dis_height / 2 - 30))
                    
                    if (self.score > self.hi_score):
                        score_file = open("score.txt", "w")
                        score_file.write(str(self.score))
                        score_file.close()
                        self.hi_score = self.score
                else:
                    # if we hit food, then draw new food
                    if (self.pos_x == self.food_x and self.pos_y == self.food_y):
                        self.draw_food()
                        self.score += 2
                    else:
                        self.snake.pop(0)
                    self.snake.append(pygame.Rect(self.pos_x, self.pos_y, self.snake_block, self.snake_block))
                    for rect in self.snake:
                        pygame.draw.rect(self.display, green, rect)
                    food = pygame.Rect(self.food_x, self.food_y, self.food_block, self.food_block)
                    pygame.draw.rect(self.display, yellow, food)
       
                pygame.display.update()
                self.clock.tick(10)

def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0)
    try:
        # doesn't even have to be reachable
        s.connect(('10.254.254.254', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

def start_udp_server():
    localIP     = get_ip()
    localPort   = 4444
    bufferSize  = 256

    # Create a datagram socket
    UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Bind to address and ip
    UDPServerSocket.bind((localIP, localPort))

    print("UDP server up and listening ({ip}, {port})".format(ip = localIP, port = localPort))

    # Listen for incoming datagrams

    while(True):
        bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
        message = bytesAddressPair[0]
        address = bytesAddressPair[1]
        clientMsg = "< {ip}:{msg}".format(ip = address, msg = message)
        print(clientMsg)
    print("Terminate UDP server.")

def main():
    proc = multiprocessing.Process(target = start_udp_server, args=())
    proc.start()

    game = Snake()
    game.game_loop()

    proc.terminate()
    pygame.quit()

    


if __name__ == "__main__":
    main()
