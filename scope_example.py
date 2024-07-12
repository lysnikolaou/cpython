def fstr(*args):
    return "".join(arg if isinstance(arg, str) else arg[0]() for arg in args)

mod_food = "burrito"
mod_v = fstr"neato {mod_food}"

class Klass:
    food = "burrito"
    v = fstr"neato {food}"

print(mod_v)  # neato burrito
print(Klass.v)  # neato burrito
