from os.path import isfile

Import("env")

env_file = isfile(".env")

if not env_file:
    print("File .env not found!")

assert env_file

try:
    f = open(".env", "r")
    lines = f.readlines()
    envs = []
    for line in lines:
        envs.append("-D {}".format(line.strip()))
    env.Append(BUILD_FLAGS=envs)
except IOError:
    print("File .env not accessible")
finally:
    f.close()
