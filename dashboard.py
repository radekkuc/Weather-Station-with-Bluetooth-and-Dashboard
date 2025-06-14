# dashboard.py

import pandas as pd
import re
from dash import Dash, dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go

LOG_PATH = "log.csv"

app = Dash(__name__)
app.title = "Live Weather Dashboard"

# Layout
app.layout = html.Div([
    html.H1("Live Weather Station Dashboard"),
    dcc.Interval(id='interval', interval=10*1000, n_intervals=0),  # every 10 seconds

    dcc.Graph(id='temperature-graph'),
    dcc.Graph(id='pressure-graph'),
    dcc.Graph(id='humidity-graph'),
    dcc.Graph(id='mq9-graph')
])


# Helper to parse log.csv into structured DataFrame
def parse_log():
    with open(LOG_PATH, encoding="utf-8", errors="replace") as f:
        df = pd.read_csv(f)

    df["timestamp"] = pd.to_datetime(df["timestamp"], errors="coerce")

    temp_data = []
    press_data = []
    hum_data = []
    mq9_data = []

def parse_log():
    with open(LOG_PATH, encoding="utf-8", errors="replace") as f:
        df = pd.read_csv(f)
    df["timestamp"] = pd.to_datetime(df["timestamp"], errors="coerce")  # also avoid bad timestamps

    temp_data = []
    press_data = []
    hum_data = []
    mq9_data = []

    for _, row in df.iterrows():
        text = row.get("payload")
        ts = row.get("timestamp")

        if pd.isna(text) or pd.isna(ts):
            continue

        if "Temperature" in text:
            m = re.search(r"([\d.]+)", text)
            if m:
                temp_data.append((ts, float(m.group(1))))
        elif "Pressure" in text:
            m = re.search(r"([\d.]+)", text)
            if m:
                press_data.append((ts, float(m.group(1))))
        elif "Humidity" in text:
            m = re.search(r"([\d.]+)", text)
            if m:
                hum_data.append((ts, float(m.group(1))))
        elif "Rs/R0" in text:
            m = re.search(r"is: ([\d.]+)", text)
            if m:
                mq9_data.append((ts, float(m.group(1))))

    return {
        "temperature": pd.DataFrame(temp_data, columns=["timestamp", "value"]),
        "pressure": pd.DataFrame(press_data, columns=["timestamp", "value"]),
        "humidity": pd.DataFrame(hum_data, columns=["timestamp", "value"]),
        "mq9": pd.DataFrame(mq9_data, columns=["timestamp", "value"])
    }



# Update graphs every 10 seconds
@app.callback(
    Output('temperature-graph', 'figure'),
    Output('pressure-graph', 'figure'),
    Output('humidity-graph', 'figure'),
    Output('mq9-graph', 'figure'),
    Input('interval', 'n_intervals')
)
def update_graphs(n):
    data = parse_log()

    def make_fig(df, title, y_label):
        return {
            "data": [go.Scatter(x=df["timestamp"], y=df["value"], mode='lines+markers')],
            "layout": go.Layout(title=title, xaxis_title="Time", yaxis_title=y_label)
        }

    return (
        make_fig(data["temperature"], "Temperature (°C)", "°C"),
        make_fig(data["pressure"], "Pressure (hPa)", "hPa"),
        make_fig(data["humidity"], "Humidity (%)", "%"),
        make_fig(data["mq9"], "Gas Concentration (ppm)", "ppm")
    )


if __name__ == '__main__':
    app.run(debug=True)
