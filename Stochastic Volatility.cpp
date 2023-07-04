#include <iostream>
#include <cmath>
#include <vector>
#include <limits>

using namespace std;

// Define the Stochastic Volatility Model parameters
double theta = 0.04; // Mean reversion level
double kappa = 1.0;  // Mean reversion speed
double sigma = 0.1;  // Volatility of volatility
double rho = -0.5;   // Correlation between asset price and volatility

// Generate random numbers from a standard normal distribution
vector<double> generateStandardNormalRandomNumbers(int numSimulations)
{
    vector<double> randomNumbers;
    for (int i = 0; i < numSimulations; ++i)
    {
        double u1 = static_cast<double>(rand()) / RAND_MAX;
        double u2 = static_cast<double>(rand()) / RAND_MAX;
        double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        randomNumbers.push_back(z0);
    }
    return randomNumbers;
}

// Simulate the stochastic volatility model using Euler's method
vector<double> simulateStochasticVolatilityModel(double T, int numSteps, int numSimulations)
{
    vector<double> randomNumbers = generateStandardNormalRandomNumbers(numSteps * numSimulations);
    vector<double> volatilityPaths(numSteps * numSimulations);
    double vol = sigma;

    for (int i = 0; i < numSteps * numSimulations; ++i)
    {
        vol += kappa * (theta - vol) * (T / numSteps) + sigma * sqrt(T / numSteps) * randomNumbers[i];
        volatilityPaths[i] = vol;
    }
    return volatilityPaths;
}

// Calculate option price using the Black-Scholes formula
double calculateOptionPrice(double S0, double K, double r, double T, double sigma)
{
    double d1 = (log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt(T));
    double d2 = d1 - sigma * sqrt(T);
    double Nd1 = 0.5 * (1.0 + erf(d1 / sqrt(2.0)));
    double Nd2 = 0.5 * (1.0 + erf(d2 / sqrt(2.0)));
    return S0 * Nd1 - K * exp(-r * T) * Nd2;
}

int main()
{
    double S0, K, r;
    int numSimulations;

    // User inputs with exception handling
    while (true)
    {
        cout << "Enter the initial stock price (S0): ";
        if (cin >> S0)
        {
            break;
        }
        else
        {
            cout << "Invalid input. Please enter a numerical value." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    while (true)
    {
        cout << "Enter the strike price (K): ";
        if (cin >> K)
        {
            break;
        }
        else
        {
            cout << "Invalid input. Please enter a numerical value." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    while (true)
    {
        cout << "Enter the risk-free interest rate (r): ";
        if (cin >> r)
        {
            break;
        }
        else
        {
            cout << "Invalid input. Please enter a numerical value." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    while (true)
    {
        cout << "Enter the number of Monte Carlo simulations: ";
        if (cin >> numSimulations)
        {
            break;
        }
        else
        {
            cout << "Invalid input. Please enter a numerical value." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    double T = 1.0;      // Total time period
    int numSteps = 252;  // Number of time steps (daily data)

    double optionPrice = 0.0;

    // Simulate the stochastic volatility model
    vector<double> volatilityPaths = simulateStochasticVolatilityModel(T, numSteps, numSimulations);

    // Calculate the option price for each path
    for (int i = 0; i < numSimulations; ++i)
    {
        double sigmaT = volatilityPaths[(i + 1) * numSteps - 1];
        optionPrice += calculateOptionPrice(S0, K, r, T, sigmaT);
    }

    // Average the option price over all simulations
    optionPrice /= numSimulations;

    cout << "Option Price: " << optionPrice << endl;

    return 0;
}