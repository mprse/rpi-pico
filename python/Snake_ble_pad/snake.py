import pygame  # Import pygame graphics library
import time # Import time module
import random # Import random module
import os # Import os module
import socket # Import socket module
import multiprocessing # Import multiprocessing module
from time import sleep # Import sleep function from time module
from ble_pad import BLE_PAD # Import ble_pad module
import asyncio  # Import asyncio module
import signal # Import signal module
import multiprocessing # Import multiprocessing module

WHITE = (255, 255, 255) # White
YELLOW = (255, 255, 102) # Yellow
BLACK = (0, 0, 0) # Black
RED = (213, 50, 80) # Red
GREEN = (0, 255, 0) # Green
BLUE = (50, 153, 213) # Blue

class Snake:
    def __init__(self) -> None:
        self.snake = []
        self.score = 0
        self.score_offset = 30
        self.display_width = 600
        self.display_height = 400 + self.score_offset
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
        self.ble_status_queue = multiprocessing.Queue()
        self.ble_cmd_queue = multiprocessing.Queue()
        self.ble_pad = BLE_PAD(self.ble_status_queue, self.ble_cmd_queue)
        self.ble_pad_process = None
        self.ble_pad_is_running = False

        # load bluetooth icon
        self.ble_image = pygame.image.load('bluetooth.png')
        self.ble_image = pygame.transform.scale(self.ble_image, (self.ble_image.get_width() / 20, self.ble_image.get_height() / 20))
        self.ble_image_visible = "off"
        self.ble_image_toggler = False
        self.ble_blink_interval = 300  # milliseconds
        self.ble_last_time = 0

        if os.path.exists("hiscore.txt"):
            with open("hiscore.txt", "r") as f:
                self.hi_score = int(f.read())

        pygame.init() # Initialize pygame

        self.status_font = pygame.font.SysFont("bahnschrift", 50)
        self.score_font = pygame.font.SysFont("comicsansms", 20)
        self.display = pygame.display.set_mode((self.display_width, self.display_height)) # Set display size
        pygame.display.set_caption("Snake v.1.1") # Set display caption
        self.clock = pygame.time.Clock() # Set clock

    def reset_snake(self):
        self.pos_x = 10
        self.pos_y = 10 + self.score_offset
        self.move_x = 10
        self.move_y = 0
        self.score = 0
        self.snake = [pygame.Rect(self.pos_x, self.pos_y, self.snake_block, self.snake_block),
            pygame.Rect(self.pos_x + self.snake_block, self.pos_y, self.snake_block, self.snake_block),
            pygame.Rect(self.pos_x + 2 * self.snake_block, self.pos_y, self.snake_block, self.snake_block)]
        self.pos_x = 30
        self.draw_food()

    def is_game_over(self):
        # check if we hit ourself
        for rect in self.snake:
            if (rect.x == self.pos_x and rect.y == self.pos_y):
                return True

        return False

    def draw_food(self):
            food_pos_ok = False

            while (not food_pos_ok):
                self.food_x = random.randrange(0, int(self.display_width / 10)) * 10
                self.food_y = random.randrange(0, int((self.display_height - self.score_offset) / 10)) * 10 + self.score_offset

                # Redraw if food is on snake
                for rect in self.snake:
                    if (rect.x == self.food_x and rect.y == self.food_y):
                        continue
                break

    def ble_pad_status(self):
        if(self.ble_status_queue.empty() == False):
            ble_status = self.ble_status_queue.get()
            if(ble_status == 1):
                #print("BLE Connecting")
                pass
            elif(ble_status == 2):
                #print("BLE Connected")
                self.ble_image_visible = "on"
            elif(ble_status == 3):
                print("BLE Disconnected")
                print("Searching for BLE PAD...")
                if self.ble_pad_is_running == True:
                    print("join 1")
                    self.ble_pad_process.join()
                    print("join 2")
                    self.ble_pad_is_running = False
                self.ble_pad_process = multiprocessing.Process(target=self.ble_pad.start)
                self.ble_pad_process.start()
                self.ble_image_visible = "blink"
                self.ble_pad_is_running = True
            elif(ble_status == 4):
                print("BLE Error")
                self.ble_image_visible = "error"
            else:
                print("Unknown BLE Status")            
        
        if(self.ble_image_visible == "on"):
            self.display.blit(self.ble_image, (self.display_width - 170, 2))
        elif(self.ble_image_visible == "blink"):
            current_time = pygame.time.get_ticks()
            if(current_time - self.ble_last_time > self.ble_blink_interval):
                self.ble_last_time = current_time
                self.ble_image_toggler = not self.ble_image_toggler
                if(self.ble_image_toggler == True):
                    self.display.blit(self.ble_image, (self.display_width - 170, 2))
                else:
                    self.display.fill(BLACK, (self.display_width - 170, 2, self.ble_image.get_width(), self.ble_image.get_height()))
        elif(self.ble_image_visible == "off"):
            self.display.fill(BLACK, (self.display_width - 170, 2, self.ble_image.get_width(), self.ble_image.get_height()))


    def game_loop(self):
        game_over = False
        game_on = True
        game_pause = False
        self.reset_snake()

        while game_on:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    game_on = False
                    if(self.ble_pad_is_running == True):
                        print("Stopping BLE Pad")
                        self.ble_pad_is_running = False
                        self.ble_cmd_queue.put("stop")
                        self.ble_pad_process.join()
                    return
                if event.type == pygame.KEYDOWN:
                    if game_over:
                        self.reset_snake()
                        game_over = False
                    if event.key == pygame.K_SPACE or event.key == pygame.K_PAUSE:
                        game_pause = False
                        pygame.event.clear()
            while (not game_over) and (not game_pause):
                # setup display
                self.display.fill(BLUE)

                score_rect = pygame.Rect(0, 0, self.display_width, self.score_offset)
                pygame.draw.rect(self.display, BLACK, score_rect)

                # Score
                score_surface = self.score_font.render("Score: " + str(self.score), False, RED)
                self.display.blit(score_surface, (10, 0))

                # Hi score
                hi_score_surface = self.score_font.render("Hi Score: " + str(self.hi_score), False, RED)
                self.display.blit(hi_score_surface, (self.display_width - 130, 0))

                # handle keyboard events and update snake position
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        game_on = False
                        game_over = True
                        if(self.ble_pad_is_running == True):
                            print("Stopping BLE Pad")
                            self.ble_pad_is_running = False
                            self.ble_cmd_queue.put("stop")
                            self.ble_pad_process.join()
                        return
                    if event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_LEFT:
                            if self.move_x != 10:
                                self.move_x = -10
                                self.move_y = 0
                        elif event.key == pygame.K_RIGHT:
                            if self.move_x != -10:
                                self.move_x = 10
                                self.move_y = 0
                        elif event.key == pygame.K_UP:
                            if self.move_y != 10:
                                self.move_x = 0
                                self.move_y = -10
                        elif event.key == pygame.K_DOWN:
                            if self.move_y != -10:
                                self.move_x = 0
                                self.move_y = 10
                        elif event.key == pygame.K_SPACE or event.key == pygame.K_PAUSE:
                            print("PAUSE1")
                            game_pause = True
                            text_surface = self.status_font.render("Game Paused", False, RED)
                            self.display.blit(text_surface, (self.display_width / 2 - 150, self.display_height / 2 - 30))


                # Update snake position
                self.pos_x += self.move_x
                self.pos_y += self.move_y

                if self.pos_x < 0:
                    self.pos_x = self.display_width - self.snake_block

                if self.pos_x > self.display_width - self.snake_block:
                    self.pos_x = 0

                if self.pos_y < self.score_offset:
                    self.pos_y = self.display_height - self.snake_block

                if self.pos_y > self.display_height - self.snake_block:
                    self.pos_y = self.score_offset

                if self.is_game_over() or game_over is True:
                    game_over = True
                    text_surface = self.status_font.render("Game Over", False, RED)
                    self.display.blit(text_surface, (self.display_width / 2 - 100, self.display_height / 2 - 30))

                    if self.score > self.hi_score:
                        self.hi_score = self.score
                        with open("hiscore.txt", "w") as f:
                            f.write(str(self.hi_score))
                else:
                    # handle eating food
                    if (self.pos_x == self.food_x and self.pos_y == self.food_y):
                        self.score += 1
                        self.ble_cmd_queue.put(self.score)
                        self.draw_food()
                    else:
                        self.snake.pop(0)

                    # draw snake
                    self.snake.append(pygame.Rect(self.pos_x, self.pos_y, self.snake_block, self.snake_block))

                    for rect in self.snake:
                        pygame.draw.rect(self.display, GREEN, rect)

                    food = pygame.Rect(self.food_x, self.food_y, self.food_block, self.food_block)
                    pygame.draw.rect(self.display, RED, food)
                
                self.ble_pad_status()
                pygame.display.update()
                self.clock.tick(10)

            self.ble_pad_status()
                
            pygame.display.update()
            self.clock.tick(10)

def main():
    snake = Snake()
    snake.game_loop()
    pygame.quit()

if __name__ == "__main__":
    main()