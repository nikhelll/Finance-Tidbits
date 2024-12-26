# breakout_analyzer.py

import yfinance as yf
import pandas as pd
import streamlit as st
import gspread
from oauth2client.service_account import ServiceAccountCredentials

# --- Fetch and Prepare Data ---
def fetch_and_prepare_data(ticker, start_date, end_date):
    """
    Fetch stock data and prepare it with additional calculations.
    """
    try:
        # Fetch stock data
        stock_data = yf.download(ticker, start=start_date, end=end_date)
        stock_data.reset_index(inplace=True)

        # Calculate 20-day rolling average volume
        stock_data['20d_avg_volume'] = stock_data['Volume'].rolling(window=20).mean()

        # Calculate daily percent change
        stock_data['daily_pct_change'] = stock_data['Close'].pct_change() * 100

        return stock_data

    except Exception as e:
        st.error(f"Error fetching data for {ticker}: {e}")
        return None


# --- Identify Breakout Days ---
def identify_breakout_days(data, volume_threshold, price_change_threshold):
    """
    Identify breakout days based on volume and price change thresholds.
    """
    breakout_days = data[
        (data['Volume'] > (volume_threshold / 100) * data['20d_avg_volume']) & 
        (data['daily_pct_change'] > price_change_threshold)
    ]
    return breakout_days


# --- Calculate Holding Period Returns ---
def calculate_holding_returns(data, breakout_days, holding_period):
    """
    Calculate holding period returns for breakout days.
    """
    returns = []
    for index, row in breakout_days.iterrows():
        breakout_index = row.name
        # Ensure there are enough days for the holding period
        holding_end_index = breakout_index + holding_period
        if holding_end_index < len(data):
            holding_return = ((data.loc[holding_end_index, 'Close'] - row['Close']) / row['Close']) * 100
        else:
            holding_return = None  # Not enough data to calculate return
        returns.append(holding_return)
    
    breakout_days.loc[:, 'holding_return'] = returns  # Use .loc to avoid SettingWithCopyWarning
    return breakout_days


# --- Generate Report ---
def generate_report(data, filename="breakout_report.csv"):
    """
    Generate a CSV report of breakout days.
    """
    data.to_csv(filename, index=False)
    st.success(f"Report saved as {filename}. You can download it below.")
    st.download_button("Download Report", data=open(filename, 'rb'), file_name=filename, mime="text/csv")


# --- Streamlit UI ---
def main():
    st.title("Stock Breakout Analyzer")
    st.write(
        """
        Analyze stock breakout signals based on volume and price thresholds, and calculate returns for a specified holding period.
        """
    )

    # User Inputs
    ticker = st.text_input("Ticker Symbol", value="AAPL")
    start_date = st.date_input("Start Date")
    end_date = st.date_input("End Date")
    volume_threshold = st.number_input("Volume Breakout Threshold (%)", value=200)
    price_change_threshold = st.number_input("Price Change Threshold (%)", value=2)
    holding_period = st.number_input("Holding Period (Days)", value=10, min_value=1, step=1)

    # Generate Report Button
    if st.button("Generate Report"):
        data = fetch_and_prepare_data(ticker, start_date, end_date)
        if data is not None:
            breakout_days = identify_breakout_days(data, volume_threshold, price_change_threshold)
            breakout_days = calculate_holding_returns(data, breakout_days, holding_period)
            
            if not breakout_days.empty:
                # Save and display report
                filename = f"{ticker}_breakout_report.csv"
                generate_report(breakout_days, filename)
                st.write("Breakout Statistics:")
                st.dataframe(breakout_days[['Date', 'Close', 'Volume', '20d_avg_volume', 'daily_pct_change', 'holding_return']])
            else:
                st.warning("No breakout signals identified based on the given parameters.")


# Run the app
if __name__ == "__main__":
    main()