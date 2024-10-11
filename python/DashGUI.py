import dash
from dash import html, dcc
from dash.dependencies import Input, Output

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
            html.H2("Linker Bereich"),
            html.P("Wähle ein Bild aus:"),
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
            html.H2("Rechter Bereich"),
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
            html.Button('Apply', id='apply-button', n_clicks=0, 
                        style={'margin-top': '20px', 'width': '100%'})  
        ]
    )
])

# Callback to update the image based on button clicks
@app.callback(
    Output('image-display', 'src'),
    [Input('button-rgb', 'n_clicks'),
     Input('button-th', 'n_clicks'),
     Input('button-dp', 'n_clicks')]
)
def update_image(n_clicks1, n_clicks2, n_clicks3):
    if n_clicks1 > n_clicks2 and n_clicks1 > n_clicks3:
        return '/assets/pose_0_rgb.png'  # Image 1
    elif n_clicks2 > n_clicks1 and n_clicks2 > n_clicks3:
        return '/assets/pose_0_thermal.png'  # Image 2
    elif n_clicks3 > n_clicks1 and n_clicks3 > n_clicks2:
        return '/assets/pose_0_depth.png'  # Image 3
    return '/assets/pose_0_rgb.png'  # Default image (RGB)

# Start the server
if __name__ == '__main__':
    app.run(debug=True)
