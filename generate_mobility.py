import random

"""
States:
  0      1       2      3       4       5
(0, 0) (0, 1) (1, 1) (0, >1) (1, >1) (>1, >1)
Connected to (new, total) networks
"""

indexStates = {0:(0, 0), 1:(0, 1), 2:(1, 1), 3:(0, 2), 4:(1, 2), 5:(2, 2)}

# first row is P(state 0, state 1, state 2, state 3, state 4, state5 | state 0)
transition_matrix =[
 [0.885777092146, 0.00000978438477419, 0.110123250634, 0.0, 0.0, 0.00408987283561],
 [0.201814845024, 0.726692495922, 0.0234910277325, 0.0, 0.0448613376835, 0.00314029363785],
 [0.647450895043, 0.283448501007, 0.0466565101475, 0.0, 0.0193699330758, 0.00307416072692],
 [0.0749542961609, 0.433272394881, 0.0100548446069, 0.424131627057, 0.0539305301645, 0.0036563071298],
 [0.173070915339, 0.594189315839, 0.0196813495783, 0.159012808497, 0.0527960012496, 0.00124960949703],
 [0.438070404172, 0.400260756193, 0.0397653194263, 0.0788787483703, 0.0397653194263, 0.00325945241199]]


NUM_USERS = 10 # must be divisable by 2
users = [0]*int(NUM_USERS/2) + [1]*int(NUM_USERS/2)
# assume half start connected, half not connected


def step(users, transition_matrix):
    new_states = []
    for u in users:
        transition_prob = transition_matrix[u] # gets the row for the user's current state
        r = random.random() # floating point between [0, 1)
        nextState = 0
        r = r - transition_prob[0]
        while r >= 0:
            nextState += 1
            r = r - transition_prob[0]
        new_states += [nextState]
    return new_states

def print_states(states):
    print([indexStates[x] for x in states])
    
            
        
        
  

 

 

                     
