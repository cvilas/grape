import grape_ipc

config = grape_ipc.Config()
config.name = "Python publisher example"
config.scope = grape_ipc.Scope.Network

grape_ipc.init(config)

# Define a match callback
def match_callback(match):
    if match.status == grape_ipc.MatchStatus.Matched:
        print("Matched with a new remote endpoint!")
    elif match.status == grape_ipc.MatchStatus.Unmatched:
        print("A previously matched endpoint is no longer available.")
    else:
        print("Unknown match status.")

# Create a Topic object (you'll need to bind Topic if not already done)
topic = grape_ipc.Topic()
topic.name = "example_topic"

# Create a Publisher with the match callback
publisher = grape_ipc.Publisher(topic, match_callback)

while grape_ipc.ok():
    publisher.publish(b"Hello, IPC!")