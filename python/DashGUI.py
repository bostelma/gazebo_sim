import sdformat13 as sdf
import gz.math7 as gzm
import dash
from dash import html, dcc
from dash.dependencies import Input, Output, State
import os
from photo_shoot_config import PhotoShootConfig
from person_config import PersonConfig
from forest_config import ForestConfig
from world_config import WorldConfig
from launcher import Launcher


# Initialize Dash App
app = dash.Dash(__name__)

layout_1 = html.Div([
    dcc.Store(id='last-button-clicked', data='rgb'),  # Default to 'rgb'
    dcc.Store(id='apply-store'),
    
    # Container for layout buttons (centered at the top)
    html.Div(
        style={
            'width': '100%', 'text-align': 'center', 'margin-bottom': '20px'
        },
        children=[
            html.Button('Forest Config', id='button-layout-1', n_clicks=0),
            html.Button('Thermal Config', id='button-layout-2', n_clicks=0),
        ]
    ),
    
    # Container for image and sliders (flexbox layout)
    html.Div(
        style={
            'display': 'flex', 'justify-content': 'space-between', 'align-items': 'flex-start', 'width': '100%',
        },
        children=[
            # Left column (image)
            html.Div(
                style={
                    'flex': '1', 'padding': '10px', 'max-width': '50%',
                },
                children=[
                    html.Div(
                        style={'position': 'relative', 'width': '100%', 'height': 'auto'},
                        children=[
                            # Insert image
                            html.Img(
                                id='image-display', 
                                src="/assets/pose_0_rgb.png",  # Default image (RGB)
                                style={'width': '100%', 'height': 'auto'},
                            ),
                            # Add buttons (positioned on top of image)
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
            
            # Right column (sliders and apply button)
            html.Div(
                style={
                    'flex': '1', 'padding': '10px', 'max-width': '50%',
                },
                children=[
                    # Sliders
                    html.Div(id='sliders-container', children=[
                        # Define all sliders here as in your original layout...
                        html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                            html.Label('clump_max', style={'width': '150px'}),
                            html.Div(style={'width': '80%'}, children=[
                                dcc.Slider(id='slider-1', min=0, max=1, step=0.05, value=0.45, 
                                           marks={i: str(i) for i in range(0, 101, 20)},
                                           tooltip={"placement": "bottom", "always_visible": True},
                                           persistence=True)
                            ])
                        ]),
                        html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('clump_min', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-2', min=0, max=1, step=0.05, value=0.4, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('length_falloff_factor', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-3', min=0, max=1, step=0.05, value=0.65, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('length_falloff_power', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-4', min=0, max=1, step=0.05, value=0.75, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('branch factor', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-5', min=0, max=5, step=0.05, value=2.45, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('radius_falloff_rate', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-6', min=0, max=1, step=0.05, value=0.7, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('climb_rate', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-7', min=0, max=1, step=0.05, value=0.55, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('taper_rate', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-8', min=0, max=1, step=0.05, value=0.8, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('twist_rate', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-9', min=0, max=10, step=0.5, value=8.0, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('segments', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-10', min=0, max=10, step=1, value=6, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('levels', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-11', min=0, max=10, step=1, value=6, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('sweep_amount', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-12', min=0, max=1, step=0.05, value=0.0, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('initial_branch_length', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-13', min=0, max=1, step=0.05, value=0.7, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('trunk_length', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-14', min=0, max=1, step=0.05, value=1, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('drop_amount', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-15', min=0, max=1, step=0.05, value=0.0, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('grow_amount', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-16', min=0, max=1, step=0.05, value=0.4, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('v_multiplier', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-17', min=0, max=1, step=0.05, value=0.2, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('twig_scale', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-18', min=0, max=1, step=0.05, value=0.2, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
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
        ]
    )
])

layout_2 = html.Div([
    dcc.Store(id='last-button-clicked', data='rgb'),  # Default to 'rgb'
    dcc.Store(id='apply-store2'),
    
    # Container for layout buttons (centered at the top)
    html.Div(
        style={
            'width': '100%', 'text-align': 'center', 'margin-bottom': '20px'
        },
        children=[
            html.Button('Forest Config', id='button-layout-1', n_clicks=0),
            html.Button('Thermal Config', id='button-layout-2', n_clicks=0),
        ]
    ),
    
    # Container for image and sliders (flexbox layout)
    html.Div(
        style={
            'display': 'flex', 'justify-content': 'space-between', 'align-items': 'flex-start', 'width': '100%',
        },
        children=[
            # Left column (image)
            html.Div(
                style={
                    'flex': '1', 'padding': '10px', 'max-width': '50%',
                },
                children=[
                    html.Div(
                        style={'position': 'relative', 'width': '100%', 'height': 'auto'},
                        children=[
                            # Insert image
                            html.Img(
                                id='image-display', 
                                src="/assets/pose_0_thermal.png",  # Default image (RGB)
                                style={'width': '100%', 'height': 'auto'},
                            ),
                        ]
                    )
                ]
            ),
            
            # Right column (sliders and apply button)
            html.Div(
                style={
                    'flex': '1', 'padding': '10px', 'max-width': '50%',
                },
                children=[
                    # Sliders
                    html.Div(id='sliders-container', children=[
                        # Define all sliders here as in your original layout...
                        html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                            html.Label('ground_temperature', style={'width': '150px'}),
                            html.Div(style={'width': '80%'}, children=[
                                dcc.Slider(id='slider-ground_temperature', min=0, max=400, step=0.05, value=288.15, 
                                           marks={i: str(i) for i in range(0, 300, 50)},
                                           tooltip={"placement": "bottom", "always_visible": True},
                                           persistence=True)
                            ])
                        ]),
                        html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('trunk_temperature', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-trunk_temperature', min=0, max=400, step=0.05, value=291.15, 
                                   marks={i: str(i) for i in range(0, 300, 50)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('twigs_temperature', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-twigs_temperature', min=0, max=400, step=0.05, value=287.15, 
                                   marks={i: str(i) for i in range(0, 300, 50)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('person_temperature', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-person_temperature', min=0, max=400, step=0.05, value=307, 
                                   marks={i: str(i) for i in range(0, 400, 50)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('direct_thermal_factor', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-direct_thermal_factor', min=0, max=100, step=0.5, value=64, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('indirect_thermal_factor', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-indirect_thermal_factor', min=0, max=100, step=0.5, value=5, 
                                   marks={i: str(i) for i in range(0, 101, 20)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('lower_thermal_threshold', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-lower_thermal_threshold', min=0, max=400, step=0.05, value=285, 
                                   marks={i: str(i) for i in range(0, 400, 50)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
                html.Div(style={'display': 'flex', 'align-items': 'center', 'margin-bottom': '10px'}, children=[
                    html.Label('upper_thermal_threshold', style={'width': '150px'}),
                    html.Div(style={'width': '80%'}, children=[
                        dcc.Slider(id='slider-upper_thermal_threshold', min=0, max=400, step=0.05, value=330, 
                                   marks={i: str(i) for i in range(0, 400, 50)},
                                   tooltip={"placement": "bottom", "always_visible": True},
                                   persistence=True)
                    ])
                ]),
            ]),
                    
                # Apply Button
                html.Div(
                    style={'text-align': 'center', 'margin-top': '20px'},
                    children=[
                        html.Button('Apply', id='button-apply-2', n_clicks=0)
                        ]
                    )
                ]
            )
        ]
    )
])
# Layout of the App
app.layout = html.Div([
    dcc.Store(id='current-layout', data='layout-1'),  # Store to track the current layout
    html.Div(id='layout-container', children=layout_1)  # Set initial layout to layout_1
])


@app.callback(
    Output('current-layout', 'data'),
    Input('button-layout-1', 'n_clicks'),
    Input('button-layout-2', 'n_clicks')
)

@app.callback(
    Output('layout-container', 'children'),
    [Input('button-layout-1', 'n_clicks'),
     Input('button-layout-2', 'n_clicks')],
    prevent_initial_call=True  # Prevent callback from being triggered on first load
)
def update_layout(n_layout_1, n_layout_2):
    ctx = dash.callback_context
    if not ctx.triggered:
        return layout_1  # Default layout on initial load

    button_id = ctx.triggered[0]['prop_id'].split('.')[0]
    if button_id == 'button-layout-1':
        return layout_1
    elif button_id == 'button-layout-2':
        return layout_2

    return layout_1  # Fallback to layout_1




@app.callback(
    Output('image-display', 'src'),
    [Input('button-rgb', 'n_clicks'),
     Input('button-th', 'n_clicks'),
     Input('button-dp', 'n_clicks')],
)
def update_last_button(n_rgb, n_th, n_dp):
    ctx = dash.callback_context
    if not ctx.triggered:
        return '/assets/pose_0_rgb.png'  # Default image

    button_id = ctx.triggered[0]['prop_id'].split('.')[0]
    if button_id == 'button-rgb':
        return '/assets/pose_0_rgb.png'
    elif button_id == 'button-th':
        return '/assets/pose_0_thermal.png'
    elif button_id == 'button-dp':
        return '/assets/pose_0_depth.png'

    return '/assets/pose_0_rgb.png'

@app.callback(
    Output('apply-store', 'data'),
    Input('button-apply', 'n_clicks'),
    [State('slider-1', 'value'),
    State('slider-2', 'value'),
    State('slider-3', 'value'),
    State('slider-4', 'value'),
    State('slider-5', 'value'),
    State('slider-6', 'value'),
    State('slider-7', 'value'),
    State('slider-8', 'value'),
    State('slider-9', 'value'),
    State('slider-10', 'value'),
    State('slider-11', 'value'),
    State('slider-12', 'value'),
    State('slider-13', 'value'),
    State('slider-14', 'value'),
    State('slider-15', 'value'),
    State('slider-16', 'value'),
    State('slider-17', 'value'),
    State('slider-18', 'value')],
    prevent_initial_call=True
)

def on_apply_button_click(n_clicks, slider1, slider2, slider3, slider4, slider5, slider6, slider7,slider8,slider9,slider10,slider11,slider12,slider13,slider14,slider15,slider16,slider17,slider18):
    if n_clicks is None or n_clicks == 0:
        return dash.no_update
    world_file_in = "../worlds/example_photo_shoot.sdf"
    world_file_out = "assets/photo_shoot.sdf"
    output_directory = "assets"

    world_config = WorldConfig()
    world_config.load(world_file_in)

    light = world_config.get_light("sun")
    light.set_visualize(False)
    light.set_direction(gzm.Vector3d(0.5, 0.5, -0.9))
    
    scene = world_config.get_scene()
    scene.set_ambient(gzm.Color(0.5, 0.5, 0.5, 1.0))

    person_config = PersonConfig()
    person_config.set_model_pose("sitting")                 # Must match a .dae mesh file
                                                            # in the respective model!
    person_config.set_temperature(307)                      # In Kelvin
    person_config.add_pose(gzm.Pose3d(0, 0, 0, 0, 0, 0))    # First three values are x, y, z coordinates 
    world_config.add_plugin(person_config)

    photo_shoot_config = PhotoShootConfig()
    photo_shoot_config.set_directory(output_directory)
    # photo_shoot_config.set_prefix("some_unique_prefix_for_each_iteration")
    photo_shoot_config.set_depth_scaling(0.0, 100.0)    # Transform the depth data to a 16Bit grayscale
                                                        # image containing the depth in cm.
    photo_shoot_config.set_save_rgb(True)               # Whether to save rgb images
    photo_shoot_config.set_save_thermal(True)           # Whether to save thermal images
    photo_shoot_config.set_save_depth(True)             # Whether to save depth images
    photo_shoot_config.set_direct_thermal_factor(64)
    photo_shoot_config.set_indirect_thermal_factor(5.0)
    photo_shoot_config.set_lower_thermal_threshold(285)
    photo_shoot_config.set_upper_thermal_threshold(330)
    photo_shoot_config.add_poses([
        gzm.Pose3d(0, 0, 35, 0.0, 1.57, 0)    # The angle looks straight down
    ])
    world_config.add_plugin(photo_shoot_config)

    # Configure the procedural forest plugin. See the respective python file
    # for the available options.
    forest_config = ForestConfig()
    forest_config.set_generate(True)
    forest_config.set_ground_texture(0)         # Have a look at the respective model for options
    forest_config.set_direct_spawning(True)     # Does not work when using the gazebo gui, but 3x faster

    forest_config.set_ground_temperature(
        288.15        # Temperature in Kelvin
    )
    # forest_config.set_ground_thermal_texture(
    #     0,              # Texture index (From the model)
    #     288.15,         # Minimal temperature in Kelvin
    #     320.0           # Maximal temperature in Kelvin
    # )

    forest_config.set_trunk_temperature(291.15) # In Kelvin
    forest_config.set_twigs_temperature(287.15) # In Kelvin

    forest_config.set_size(100)                 # Width / height of the forest
    forest_config.set_texture_size(10)          # Width / height of the ground texture
    forest_config.set_trees(200)                # Number of trees
    forest_config.set_seed(100)    
    forest_config.set_species("Birch", {
        "percentage": 1.0,                      # Percentage that this species makes up of all trees
        "homogeneity": 0.95,
        "trunk_texture": 0,                     # Have a look at the respective model for options
        "twigs_texture": 0,                     # Have a look at the respective model for options
        "tree_properties": {
            "clump_max": slider1,
            "clump_min": slider2,
            "length_falloff_factor": slider3,
            "length_falloff_power": slider4,
            "branch_factor": slider5,
            "radius_falloff_rate": slider6,
            "climb_rate": slider7,
            "taper_rate": slider8,
            "twist_rate": slider9,
            "segments": slider10,
            "levels": slider11,
            "sweep_amount": slider12,
            "initial_branch_length": slider13,
            "trunk_length": slider14,
            "drop_amount": slider15,
            "grow_amount": slider16,
            "v_multiplier": slider17,
            "twig_scale": slider18
        }
    })
    world_config.add_plugin(forest_config)
    world_config.save(world_file_out)
    launcher = Launcher()
    launcher.set_launch_config("server_only", True)
    launcher.set_launch_config("running", True)
    launcher.set_launch_config("iterations", 2)
    launcher.set_launch_config("world", world_file_out)
    print(launcher.launch())
    print("done")


    return {"status": "apply_button_clicked"}


@app.callback(
    Output('apply-store2', 'data'),
    Input('button-apply-2', 'n_clicks'),
    [State('slider-ground_temperature', 'value'),
    State('slider-trunk_temperature', 'value'),
    State('slider-twigs_temperature', 'value'),
    State('slider-person_temperature', 'value'),
    State('slider-direct_thermal_factor', 'value'),
    State('slider-indirect_thermal_factor', 'value'),
    State('slider-lower_thermal_threshold', 'value'),
    State('slider-upper_thermal_threshold', 'value')],
    prevent_initial_call=True
)
def on_apply2_button_click(n_clicks, ground_tp, trunk_tp, twigs_tp, person_tp, direct_thermal_factor, indirect_thermal_factor, lower_thermal_threshold, upper_thermal_threshold):

    

    world_file_in = "../worlds/example_photo_shoot.sdf"
    world_file_out = "assets/photo_shoot_thermal.sdf"
    output_directory = "assets"

    # Start off by loading an existing world config
    # so that we don't have to create everything!
    world_config = WorldConfig()
    world_config.load(world_file_in)
    
    # Configure the sun as the light source
    # https://github.com/gazebosim/sdformat/blob/sdf13/python/src/sdf/pyLight.cc
    # shows the available functions.
    light = world_config.get_light("sun")
    light.set_visualize(False)
    light.set_direction(gzm.Vector3d(0.5, 0.5, -0.9))
    
    # Configure the scene, which holds for example the ambient light
    # https://github.com/gazebosim/sdformat/blob/sdf13/python/src/sdf/pyLight.cc
    # shows the available functions
    scene = world_config.get_scene()
    scene.set_ambient(gzm.Color(0.5, 0.5, 0.5, 1.0))

    # Configure the person plugin. See the respective python file 'person_config'
    # for the available options.
    person_config = PersonConfig()
    person_config.set_model_pose("sitting")                 # Must match a .dae mesh file
                                                            # in the respective model!
    person_config.set_temperature(person_tp)                      # In Kelvin
    person_config.add_pose(gzm.Pose3d(0, 0, 0, 0, 0, 0))    # First three values are x, y, z coordinates 
    world_config.add_plugin(person_config)

    # Configure the photo shoot plugin. See the respective python file 
    # 'photo_shoot_config.py' for the available options.
    photo_shoot_config = PhotoShootConfig()
    photo_shoot_config.set_directory(output_directory)
    # photo_shoot_config.set_prefix("some_unique_prefix_for_each_iteration")
    photo_shoot_config.set_depth_scaling(0.0, 100.0)    # Transform the depth data to a 16Bit grayscale
                                                        # image containing the depth in cm.
    photo_shoot_config.set_save_rgb(True)               # Whether to save rgb images
    photo_shoot_config.set_save_thermal(True)           # Whether to save thermal images
    photo_shoot_config.set_save_depth(True)             # Whether to save depth images
    photo_shoot_config.set_direct_thermal_factor(direct_thermal_factor)
    photo_shoot_config.set_indirect_thermal_factor(indirect_thermal_factor)
    photo_shoot_config.set_lower_thermal_threshold(lower_thermal_threshold)
    photo_shoot_config.set_upper_thermal_threshold(upper_thermal_threshold)
    photo_shoot_config.add_poses([
        gzm.Pose3d(0, 0, 35, 0.0, 1.57, 0)    # The angle looks straight down
    ])
    world_config.add_plugin(photo_shoot_config)

    # Configure the procedural forest plugin. See the respective python file
    # for the available options.
    forest_config = ForestConfig()
    forest_config.set_generate(True)
    forest_config.set_ground_texture(0)         # Have a look at the respective model for options
    forest_config.set_direct_spawning(True)     # Does not work when using the gazebo gui, but 3x faster

    # Set the ground temperature, either by setting a constant temperature or
    # by setting a texture that represents the temperature. The texture needs
    # a minimal and maximal temperature, which are used to map the texture to
    # the temperature range.
    forest_config.set_ground_temperature(
        ground_tp      # Temperature in Kelvin
    )
    # forest_config.set_ground_thermal_texture(
    #     0,              # Texture index (From the model)
    #     288.15,         # Minimal temperature in Kelvin
    #     320.0           # Maximal temperature in Kelvin
    # )

    forest_config.set_trunk_temperature(trunk_tp) # In Kelvin
    forest_config.set_twigs_temperature(twigs_tp) # In Kelvin

    forest_config.set_size(100)                 # Width / height of the forest
    forest_config.set_texture_size(10)          # Width / height of the ground texture
    forest_config.set_trees(200)                # Number of trees
    forest_config.set_seed(100)                 # Change the seed for multiple runs!
    forest_config.set_species("Birch", {
        "percentage": 1.0,                      # Percentage that this species makes up of all trees
        "homogeneity": 0.95,
        "trunk_texture": 0,                     # Have a look at the respective model for options
        "twigs_texture": 0,                     # Have a look at the respective model for options
        "tree_properties": {
            "clump_max": 0.45,
            "clump_min": 0.4,
            "length_falloff_factor": 0.65,
            "length_falloff_power": 0.75,
            "branch_factor": 2.45,
            "radius_falloff_rate": 0.7,
            "climb_rate": 0.55,
            "taper_rate": 0.8,
            "twist_rate": 8.0,
            "segments": 6,
            "levels": 6,
            "sweep_amount": 0.0,
            "initial_branch_length": 0.7,
            "trunk_length": 1.0,
            "drop_amount": 0.0,
            "grow_amount": 0.4,
            "v_multiplier": 0.2,
            "twig_scale": 0.2
        }
    })
    world_config.add_plugin(forest_config)

    # Save the modified config
    world_config.save(world_file_out)

    # Launch the simulation
    launcher = Launcher()
    launcher.set_launch_config("server_only", True)
    launcher.set_launch_config("running", True)
    launcher.set_launch_config("iterations", 2)
    launcher.set_launch_config("world", world_file_out)
    print(launcher.launch())
    return {"status": "apply_button2_clicked"}

# Run the app
if __name__ == '__main__':
    app.run_server(debug=True)
