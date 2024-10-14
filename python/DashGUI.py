import dash
from dash import html, dcc
from dash.dependencies import Input, Output, State
import pandas as pd
import os

# Initialize Dash App
app = dash.Dash(__name__)

# Layout of the App
app.layout = html.Div([
    # Left column with image and buttons
    html.Div(
        style={
            'width': '45%', 'display': 'inline-block', 'vertical-align': 'top', 
            'padding': '10px', 'border-right': '2px solid black', 'position': 'relative'
        },
        children=[
            # Container for image
            html.Div(
                style={'position': 'relative', 'width': '100%', 'height': 'auto'},
                children=[
                    # Insert image
                    html.Img(
                        id='image-display', 
                        src="/assets/pose_0_rgb.png",  # Default image (RGB)
                        style={'width': '100%', 'height': 'auto'}
                    ),
                    # Add buttons
                    html.Div(
                        style={
                            'position': 'absolute', 
                            'top': '10px', 
                            'left': '10px', 
                            'display': 'flex',  
                            'gap': '5px'  
                        },
                        children=[
                            html.Button('RGB', id='button-rgb', n_clicks=0),
                            html.Button('Thermal', id='button-th', n_clicks=0),
                            html.Button('Tiefe', id='button-dp', n_clicks=0)
                        ]
                    )
                ]
            )
        ]
    ),
    
    # Right column with dropdowns and sliders
    html.Div(
        style={
            'width': '45%', 'display': 'inline-block', 'vertical-align': 'top', 
            'padding': '10px'
        },
        children=[
            html.Div(
                style={'display': 'flex', 'justify-content': 'center', 'gap': '10px', 'margin-bottom': '20px'},
                children=[
                    html.Button('RGB', id='button-rgb-right', n_clicks=0),
                    html.Button('Thermal', id='button-th-right', n_clicks=0),
                    html.Button('Tiefe', id='button-dp-right', n_clicks=0)
                ]
            ),
            # Dropdowns
            html.Div(id='dropdowns-container', children=[
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Dropdown 1', style={'width': '150px'}),
                    dcc.Dropdown(
                        id='dropdown-1',
                        options=[
                            {'label': 'Option 1', 'value': 'opt1'},
                            {'label': 'Option 2', 'value': 'opt2'},
                            {'label': 'Option 3', 'value': 'opt3'}
                        ],
                        value='opt1',
                        style={'margin-left': '10px', 'width': '150px'}
                    )
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Dropdown 2', style={'width': '150px'}),
                    dcc.Dropdown(
                        id='dropdown-2',
                        options=[
                            {'label': 'Option 1', 'value': 'opt1'},
                            {'label': 'Option 2', 'value': 'opt2'},
                            {'label': 'Option 3', 'value': 'opt3'}
                        ],
                        value='opt1',
                        style={'margin-left': '10px', 'width': '150px'}
                    )
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Dropdown 3', style={'width': '150px'}),
                    dcc.Dropdown(
                        id='dropdown-3',
                        options=[
                            {'label': 'Option 1', 'value': 'opt1'},
                            {'label': 'Option 2', 'value': 'opt2'},
                            {'label': 'Option 3', 'value': 'opt3'}
                        ],
                        value='opt1',
                        style={'margin-left': '10px', 'width': '150px'}
                    )
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Dropdown 4', style={'width': '150px'}),
                    dcc.Dropdown(
                        id='dropdown-4',
                        options=[
                            {'label': 'Option 1', 'value': 'opt1'},
                            {'label': 'Option 2', 'value': 'opt2'},
                            {'label': 'Option 3', 'value': 'opt3'}
                        ],
                        value='opt1',
                        style={'margin-left': '10px', 'width': '150px'}
                    )
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Dropdown 5', style={'width': '150px'}),
                    dcc.Dropdown(
                        id='dropdown-5',
                        options=[
                            {'label': 'Option 1', 'value': 'opt1'},
                            {'label': 'Option 2', 'value': 'opt2'},
                            {'label': 'Option 3', 'value': 'opt3'}
                        ],
                        value='opt1',
                        style={'margin-left': '10px', 'width': '150px'}
                    )
                ])
            ]),
            # Sliders
            html.Div(id='sliders-container', children=[
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Schieberegler 1', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-1', min=0, max=100, step=1, value=50, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True})
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Schieberegler 2', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-2', min=0, max=100, step=1, value=50, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True})
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Schieberegler 3', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-3', min=0, max=100, step=1, value=50, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True})
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Schieberegler 4', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-4', min=0, max=100, step=1, value=50, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True})
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('Schieberegler 5', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-5', min=0, max=100, step=1, value=50, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True})
                    ])
                ])
            ]),
            # Apply Button
            html.Div(
                style={'text-align': 'center', 'margin-top': '20px'},
                children=[
                    html.Button('Apply', id='button-apply', n_clicks=0)
                ]
            )
        ]
    )
])

# Function to read config from files
def read_config(file_name):
    config = {}
    file_path = os.path.join('assets', file_name)  # Update the path to include 'assets/'
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('#') or line.strip() == '':
                continue  # Skip comments and empty lines
            key, value = line.strip().split('=')
            config[key] = value
    return config

# Callbacks for button clicks to load configs
@app.callback(
    [Output('dropdown-1', 'value'),
     Output('dropdown-2', 'value'),
     Output('dropdown-3', 'value'),
     Output('dropdown-4', 'value'),
     Output('dropdown-5', 'value'),
     Output('slider-1', 'value'),
     Output('slider-2', 'value'),
     Output('slider-3', 'value'),
     Output('slider-4', 'value'),
     Output('slider-5', 'value'),
     Output('image-display', 'src')],  # Output for image source
    [Input('button-rgb', 'n_clicks'),
     Input('button-th', 'n_clicks'),
     Input('button-dp', 'n_clicks'),
     Input('button-rgb-right', 'n_clicks'),
     Input('button-th-right', 'n_clicks'),
     Input('button-dp-right', 'n_clicks')]
)
def load_config(n_rgb, n_th, n_dp, n_rgb_right, n_th_right, n_dp_right):
    ctx = dash.callback_context

    if not ctx.triggered:
        return dash.no_update

    button_id = ctx.triggered[0]['prop_id'].split('.')[0]

    # Initialize default values
    config = {
        'dropdown-1': 'opt1',
        'dropdown-2': 'opt1',
        'dropdown-3': 'opt1',
        'dropdown-4': 'opt1',
        'dropdown-5': 'opt1',
        'slider-1': 50,
        'slider-2': 50,
        'slider-3': 50,
        'slider-4': 50,
        'slider-5': 50,
        'image_src': '/assets/pose_0_rgb.png'  # Default image
    }

    # Load config based on which button was clicked
    if 'rgb' in button_id:
        config = read_config('config_rgb.txt')
        image_src = '/assets/pose_0_rgb.png'  # Image for RGB
    elif 'th' in button_id:
        config = read_config('config_thermal.txt')
        image_src = '/assets/pose_0_thermal.png'  # Image for Thermal
    elif 'dp' in button_id:
        config = read_config('config_depth.txt')
        image_src = '/assets/pose_0_depth.png'  # Image for Depth
    else:
        return dash.no_update

    # Update dropdown and slider values based on the loaded config
    return (
        config.get('dropdown-1', 'opt1'),
        config.get('dropdown-2', 'opt1'),
        config.get('dropdown-3', 'opt1'),
        config.get('dropdown-4', 'opt1'),
        config.get('dropdown-5', 'opt1'),
        int(config.get('slider-1', 50)),
        int(config.get('slider-2', 50)),
        int(config.get('slider-3', 50)),
        int(config.get('slider-4', 50)),
        int(config.get('slider-5', 50)),
        image_src  # Set the image source
    )

# Run the app
if __name__ == '__main__':
    app.run_server(debug=True)
