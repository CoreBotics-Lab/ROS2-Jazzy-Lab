import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import random
import math

def calculate_gaussian(x, mean, variance):
    """
    Standard Mathematical Formula for a Gaussian (Normal) Distribution.
    
    Formula:
        f(x) = (1 / (sigma * sqrt(2 * pi))) * exp(-((x - mean)^2) / (2 * variance))
    """
    sigma = math.sqrt(variance)
    coefficient = 1.0 / (sigma * math.sqrt(2 * math.pi))
    exponent = math.exp(-((x - mean) ** 2) / (2 * variance))
    return coefficient * exponent

class CoinFlipAnimator:
    def __init__(self):
        self.ani = None
        # Experiment setup configurations
        self.flips_per_experiment = 10
        self.theoretical_mean = 5.0
        self.theoretical_variance = 2.5
        
        # This list will accumulate our raw trial results over time
        self.all_results = []
        
        # Setup the graphics plotting layout window
        self.fig, self.ax = plt.subplots(figsize=(10, 6))
        
        # Pre-calculate the smooth theoretical Gaussian curve path
        self.x_smooth = np.linspace(0, 10, 400)
        self.gaussian_curve = [calculate_gaussian(x, self.theoretical_mean, self.theoretical_variance) for x in self.x_smooth]

    def update_frame(self, frame_number):
        """
        This function triggers repeatedly to animate the data accumulation live.
        """
        # Step 1: Control the growth rate of our sample size per frame
        if frame_number < 20:
            samples_to_add = 5      # Start slow so you can see the initial pure chaos
        elif frame_number < 50:
            samples_to_add = 50     # Speed up as the graph starts taking shape
        else:
            samples_to_add = 250    # Blaze through to finish the 10,000 trial limit

        # Step 2: Run the random coin flips for this specific frame step
        for _ in range(samples_to_add):
            # 1 = Heads, 0 = Tails
            flips = [random.choice([0, 1]) for _ in range(self.flips_per_experiment)]
            self.all_results.append(sum(flips))

        current_total_samples = len(self.all_results)

        # Step 3: Clear the previous frame drawing to prevent overlap ghosting
        self.ax.clear()
        
        # Step 4: Draw the live experiment bars
        bins = np.arange(-0.5, 11.5, 1)
        self.ax.hist(self.all_results, bins=bins, density=True, rwidth=0.8,
                     color='skyblue', edgecolor='black', alpha=0.7, 
                     label=f'Experimental Trials: {current_total_samples:,}')

        # Step 5: Draw the static ideal reference lines over the top
        self.ax.plot(self.x_smooth, self.gaussian_curve, color='red', linewidth=3, 
                     label='Theoretical Gaussian Curve Target')
        self.ax.axvline(x=5.0, color='red', linestyle=':', alpha=0.8, label='Ideal Mean Peak (5 Heads)')

        # Step 6: Maintain consistent layout scaling so the window doesn't bounce around
        self.ax.set_title('Watch Live: Chaos Transforming into a Gaussian Curve', fontsize=14, fontweight='bold')
        self.ax.set_xlabel('Number of Heads Obtained in 10 Flips', fontsize=12)
        self.ax.set_ylabel('Probability Density (Frequency Percentage)', fontsize=12)
        self.ax.set_xlim(-1, 11)
        self.ax.set_ylim(0, 0.30) # Lock the Y axis to see the bars rise up to the target line
        self.ax.set_xticks(range(11))
        self.ax.grid(axis='y', linestyle=':', alpha=0.6)
        self.ax.legend(fontsize=11, loc='upper right')

        # Automatically stop the animation loop once we break our 10,000 threshold limit
        if current_total_samples >= 10000:
            if self.ani is not None:
                self.ani.event_source.stop()
                print("Animation Complete! Target cap of 10,000 trials reached.")

    def start(self):
        # Trigger the matplotlib animation handler loop window interface natively
        # interval=50 means it refreshes every 50 milliseconds
        self.ani = animation.FuncAnimation(self.fig, self.update_frame, frames=100, interval=50, repeat=False)
        plt.show()

if __name__ == '__main__':
    animator = CoinFlipAnimator()
    animator.start()