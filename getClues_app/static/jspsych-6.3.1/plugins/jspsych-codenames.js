/**
 * jspsych-survey-multi-select
 * a jspsych plugin for codenames game
 *
 * documentation: docs.jspsych.org
 *
 */


jsPsych.plugins['codenames'] = (function() {
    var plugin = {};
  
    const LIMIT = 25;
    const WORDS = ["AFRICA", "AGENT", "AIR", "ALIEN", "ALPS", "AMAZON", "AMBULANCE",
      "AMERICA", "ANGEL", "ANTARCTICA", "APPLE", "ARM", "ATLANTIS", "AUSTRALIA", "AZTEC",
      "BACK", "BALL", "BAND", "BANK", "BAR", "BARK", "BAT", "BATTERY", "BEACH", "BEAR",
      "BEAT", "BED", "BEIJING", "BELL", "BELT", "BERLIN", "BERMUDA", "BERRY", "BILL", "BLOCK",
      "BOARD", "BOLT", "BOMB", "BOND", "BOOM", "BOOT", "BOTTLE", "BOW", "BOX", "BRIDGE",
      "BRUSH", "BUCK", "BUFFALO", "BUG", "BUGLE", "BUTTON", "CALF", "CANADA", "CAP", "CAPITAL",
      "CAR", "CARD", "CARROT", "CASINO", "CAST", "CAT", "CELL", "CENTAUR", "CENTER", "CHAIR",
      "CHANGE", "CHARGE", "CHECK", "CHEST", "CHICK", "CHINA", "CHOCOLATE", "CHURCH", "CIRCLE",
      "CLIFF", "CLOAK", "CLUB", "CODE", "COLD", "COMIC", "COMPOUND", "CONCERT", "CONDUCTOR",
      "CONTRACT", "COOK", "COPPER", "COTTON", "COURT", "COVER", "CRANE", "CRASH", "CRICKET",
      "CROSS", "CROWN", "CYCLE", "CZECH", "DANCE", "DATE", "DAY", "DEATH", "DECK", "DEGREE",
      "DIAMOND", "DICE", "DINOSAUR", "DISEASE", "DOCTOR", "DOG", "DRAFT", "DRAGON", "DRESS",
      "DRILL", "DROP", "DUCK", "DWARF", "EAGLE", "EGYPT", "EMBASSY", "ENGINE", "ENGLAND",
      "EUROPE", "EYE", "FACE", "FAIR", "FALL", "FAN", "FENCE", "FIELD", "FIGHTER", "FIGURE",
      "FILE", "FILM", "FIRE", "FISH", "FLUTE", "FLY", "FOOT", "FORCE", "FOREST", "FORK",
      "FRANCE", "GAME", "GAS", "GENIUS", "GERMANY", "GHOST", "GIANT", "GLASS", "GLOVE", "GOLD",
      "GRACE", "GRASS", "GREECE", "GREEN", "GROUND", "HAM", "HAND", "HAWK", "HEAD", "HEART",
      "HELICOPTER", "HIMALAYAS", "HOLE", "HOLLYWOOD", "HONEY", "HOOD", "HOOK", "HORN", "HORSE",
      "HORSESHOE", "HOSPITAL", "HOTEL", "ICE", "ICE CREAM", "INDIA", "IRON", "IVORY", "JACK",
      "JAM", "JET", "JUPITER", "KANGAROO", "KETCHUP", "KEY", "KID", "KING", "KIWI", "KNIFE",
      "KNIGHT", "LAB", "LAP", "LASER", "LAWYER", "LEAD", "LEMON", "LEPRECHAUN", "LIFE", "LIGHT",
      "LIMOUSINE", "LINE", "LINK", "LION", "LITTER", "LOCH NESS", "LOCK", "LOG", "LONDON",
      "LUCK", "MAIL", "MAMMOTH", "MAPLE", "MARBLE", "MARCH", "MASS", "MATCH", "MERCURY",
      "MEXICO", "MICROSCOPE", "MILLIONAIRE", "MINE", "MINT", "MISSILE", "MODEL", "MOLE", "MOON",
      "MOSCOW", "MOUNT", "MOUSE", "MOUTH", "MUG", "NAIL", "NEEDLE", "NET", "NEW YORK", "NIGHT",
      "NINJA", "NOTE", "NOVEL", "NURSE", "NUT", "OCTOPUS", "OIL", "OLIVE", "OLYMPUS", "OPERA",
      "ORANGE", "ORGAN", "PALM", "PAN", "PANTS", "PAPER", "PARACHUTE", "PARK", "PART", "PASS",
      "PASTE", "PENGUIN", "PHOENIX", "PIANO", "PIE", "PILOT", "PIN", "PIPE", "PIRATE", "PISTOL",
      "PIT", "PITCH", "PLANE", "PLASTIC", "PLATE", "PLATYPUS", "PLAY", "PLOT", "POINT",
      "POISON", "POLE", "POLICE", "POOL", "PORT", "POST", "POUND", "PRESS", "PRINCESS",
      "PUMPKIN", "PUPIL", "PYRAMID", "QUEEN", "RABBIT", "RACKET", "RAY", "REVOLUTION", "RING",
      "ROBIN", "ROBOT", "ROCK", "ROME", "ROOT", "ROSE", "ROULETTE", "ROUND", "ROW", "RULER",
      "SATELLITE", "SATURN", "SCALE", "SCHOOL", "SCIENTIST", "SCORPION", "SCREEN",
      "SCUBA DIVER", "SEAL", "SERVER", "SHADOW", "SHAKESPEARE", "SHARK", "SHIP", "SHOE", "SHOP",
      "SHOT", "SINK", "SKYSCRAPER", "SLIP", "SLUG", "SMUGGLER", "SNOW", "SNOWMAN", "SOCK",
      "SOLDIER", "SOUL", "SOUND", "SPACE", "SPELL", "SPIDER", "SPIKE", "SPINE", "SPOT",
      "SPRING", "SPY", "SQUARE", "STADIUM", "STAFF", "STAR", "STATE", "STICK", "STOCK", "STRAW",
      "STREAM", "STRIKE", "STRING", "SUB", "SUIT", "SUPERHERO", "SWING", "SWITCH", "TABLE",
      "TABLET", "TAG", "TAIL", "TAP", "TEACHER", "TELESCOPE", "TEMPLE", "THEATER", "THIEF",
      "THUMB", "TICK", "TIE", "TIME", "TOKYO", "TOOTH", "TORCH", "TOWER", "TRACK", "TRAIN",
      "TRIANGLE", "TRIP", "TRUNK", "TUBE", "TURKEY", "UNDERTAKER", "UNICORN", "VACUUM", "VAN",
      "VET", "WAKE", "WALL", "WAR", "WASHER", "WASHINGTON", "WATCH", "WATER", "WAVE", "WEB",
      "WELL", "WHALE", "WHIP", "WIND", "WITCH", "WORM", "YARD"];

    plugin.info = {
      name: 'codenames',
      description: '',
      parameters: {
        cards: {
          type: jsPsych.plugins.parameterType.COMPLEX,
          array: true,
          pretty_name: 'Cards',
          nested: {
            label: {
              type: jsPsych.plugins.parameterType.STRING,
              pretty_name: 'Label',
              default: undefined,
              description: 'The label for a card.'
            },
            team: {
              type: jsPsych.plugins.parameterType.STRING,
              pretty_name: 'Team',
              default: undefined,
              description: 'The team associated with a card.'
            },
          }
        },
        preamble: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Preamble',
          default: null,
          description: 'HTML formatted string to display at the top of the page above all the questions.'
        },
        button_label: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Button label',
          default:  'Submit',
          description: 'Label of the button.'
        },
        required_message: {
          type: jsPsych.plugins.parameterType.STRING,
          pretty_name: 'Required message',
          default: "Please select at least one of your team's cards and enter a corresponding clue",
          description: 'Message that will be displayed if required question is not answered.'
        },
        autocomplete: {
          type: jsPsych.plugins.parameterType.BOOL,
          pretty_name: 'Allow autocomplete',
          default: false,
          description: "Setting this to true will enable browser auto-complete or auto-fill for the form."
        }
      }
    }
    plugin.trial = function(display_element, trial) {
      var plugin_id_name = "jspsych-codenames";
      var plugin_id_selector = '#' + plugin_id_name;
      var _join = function( /*args*/ ) {
        var arr = Array.prototype.slice.call(arguments, _join.length);
        return arr.join(separator = '-');
      }
  

      console.log("ok");
      // inject CSS for trial
      var cssstr = ".jspsych-survey-multi-select-question { margin-top: 2em; margin-bottom: 2em; text-align: left; }"+
        ".jspsych-survey-multi-select-text span.required {color: darkred;}"+
        ".jspsych-survey-multi-select-horizontal .jspsych-survey-multi-select-text {  text-align: center;}"+
        ".jspsych-survey-multi-select-option { line-height: 2; }"+
        ".jspsych-survey-multi-select-horizontal .jspsych-survey-multi-select-option {  display: inline-block;  margin-left: 1em;  margin-right: 1em;  vertical-align: top;}"+
        "label.jspsych-survey-multi-select-text input[type='checkbox'] {margin-right: 1em;}"
      display_element.innerHTML = '<style id="jspsych-survey-multi-select-css">' + cssstr + '</style>';
    
      // form element
      var trial_form_id = _join(plugin_id_name, "form");
      display_element.innerHTML += '<form id="'+trial_form_id+'"></form>';
      var trial_form = display_element.querySelector("#" + trial_form_id);
      if ( !trial.autocomplete ) {
          trial_form.setAttribute('autocomplete',"off");
      }
      // show preamble text
      var preamble_id_name = _join(plugin_id_name, 'preamble');
      if(trial.preamble !== null){
        trial_form.innerHTML += '<div id="'+preamble_id_name+'" class="'+preamble_id_name+'">'+trial.preamble+'</div>';
      }

      var prng = 1;
      var firstPlayer = determineFirstPlayer(prng);
      //console.log(renderBoard(prng, firstPlayer));
      trial_form.innerHTML += renderBoard(prng, firstPlayer);

      // generate question order. this is randomized here as opposed to randomizing the order of trial.questions
      // so that the data are always associated with the same question regardless of order
      var card_order = [];
      for(var i=0; i<trial.cards.length; i++){
        card_order.push(i);
      }
      if(trial.randomize_card_order){
        card_order = jsPsych.randomization.shuffle(card_order);
      }
      // add multiple-select questions
      for (var i = 0; i < trial.cards.length; i++) {
        var card = trial.cards[card_order[i]];
        var card_id = card_order[i];
        // create question container
        var question_classes = [_join(plugin_id_name, 'card')];
  
        trial_form.innerHTML += '<div id="'+_join(plugin_id_name, card_id)+'" data-name="'+card.name+'" class="'+question_classes.join(' ')+'"></div>';
  
        var card_selector = _join(plugin_id_selector, card_id);
  
        // add question text
        display_element.querySelector(card_selector).innerHTML += '<p id="survey-question" class="' + plugin_id_name + '-text survey-multi-select">' + card.label + '</p>';
  

        // create option check boxes
        var option_id_name = _join(plugin_id_name, "option", card_id);
  
        // add check box container
        display_element.querySelector(card_selector).innerHTML += '<div id="'+option_id_name+'" class="'+_join(plugin_id_name, 'option')+'"></div>';
  
        // add label and question text
        var form = document.getElementById(option_id_name)
        var input_name = _join(plugin_id_name, 'response', card_id);
        var input_id = _join(plugin_id_name, 'response', card_id);
        var label = document.createElement('label');
        label.setAttribute('class', plugin_id_name+'-text');
        label.innerHTML = card.label;
        label.setAttribute('for', input_id)
        
        //var t = document.createElement('hi');
        //t.innerHTML = 'hi';
        //t.style.display = "block";
        //var el = document.createElement(type="checkbox" id="xxx" name="xxx" onclick="calc();");
        //el.innerHTML = <input type="checkbox" id="xxx" name="xxx" onclick="calc();"/>;
        display_element.querySelector(card_selector).innerHTML += '<p id="survey-question" class="' + plugin_id_name + '-text survey-multi-select">' + card.label + '</p>';

        // create checkboxes
        var input = document.createElement('input');
        input.setAttribute('type', "checkbox");
        input.setAttribute('name', input_name);
        input.setAttribute('id', input_id);
        input.setAttribute('value', card.label)
        form.appendChild(label) //prints question
        label.insertBefore(input, label.firstChild) //prints box
      }
      // add submit button
      trial_form.innerHTML += '<div class="fail-message"></div>'
      trial_form.innerHTML += '<button id="'+plugin_id_name+'-next" class="'+plugin_id_name+' jspsych-btn">'+trial.button_label+'</button>';
      
      function shuffle(prng, arr) {
        var len = arr.length;
        for (var i = 0; i < len; i++) {
          var rand_index = Math.floor((i + 1) * prng);
          var tmp = arr[rand_index];
          arr[rand_index] = arr[i];
          arr[i] = tmp;
        }
        return arr;
      }
      
      function loadWords(prng) {
        // shuffle + slice to guarantee non-repetition
        var wordlist = shuffle(prng, WORDS.slice(0));
        return wordlist.slice(0, LIMIT);
      }
      
      function determineFirstPlayer(prng) {
        if (prng < 0.5) {
          return "blue";
        } else {
          return "red";
        }
      }
      
      function wordDistribution(prng) {
        var word_indices = shuffle(prng, [...Array(25).keys()]);
        return [word_indices.slice(0, 8), word_indices.slice(8, 17), word_indices[17]];
      }
      
      function newSeed() { return Math.floor(Math.random() * 10000); }
      function getSeed() {
        var params = (new URL(window.location)).searchParams;
        return params.get("gameID") || newSeed();
      }
      
      function updateRemainingIndicators() {
        var countBlue = document.querySelectorAll(".blueword").length;
        var countRed = document.querySelectorAll(".redword").length;
        var countBlueSelected = document.querySelectorAll(".blueword.activated").length;
        var countRedSelected = document.querySelectorAll(".redword.activated").length;
      
        var target = document.getElementById("blueRemaining");
        target.innerHTML = (countBlue - countBlueSelected).toString();
      
        target = document.getElementById("redRemaining");
        target.innerHTML = (countRed - countRedSelected).toString();
      }
      
      function clickActivates(node) {
        node.addEventListener("click", function(evt) {
          evt.stopPropagation();
      
          if (window.confirm("Are you sure you want to select " + node.innerHTML + "?")) {
            node.classList.add("activated");
            updateRemainingIndicators()
          }
        });
      }
      
      //display board with all words
      function renderBoard(prng, firstPlayer) {
        var wordlist = loadWords(prng);
        var dist = wordDistribution(prng);
      
        if (firstPlayer == "red") {
          var bluewords = dist[0], redwords = dist[1];
        } else {
          var redwords = dist[0], bluewords = dist[1];
        }
        var poisonword = dist[2];
      
        var target = document.createElement('wordList');
        //var target = document.getElementById("wordList");
        // clear all children
        while (target.firstChild) {
          target.removeChild(target.firstChild);
        }
        // render the board
        var table = document.createElement("TABLE");
        for (var i = 0; i < 5; i++) {
          var tr = document.createElement("TR");
          for (var j = 0; j < 5; j++) {
            var idx = 5 * i + j;
            var td = document.createElement("TD");
            td.appendChild(document.createTextNode(wordlist[idx]));
            td.classList.add("gameword");
            if (idx == poisonword) {
              td.classList.add("poisonword")
            } else if (redwords.includes(idx)) {
              td.classList.add("redword")
            } else if (bluewords.includes(idx)) {
              td.classList.add("blueword")
            } else {
              td.classList.add("neutralword")
            }
            clickActivates(td);
            tr.appendChild(td);
          }
          table.appendChild(tr);
        }
        target.appendChild(table);
        return table;
      }
      
      function renderGameLink(gameID) {
        var href = window.location.protocol + '//' + window.location.host + window.location.pathname + '?gameID=' + gameID.toString();
      
        var target = document.getElementById("gameLink");
        target.href = href;
      
        target = document.getElementById("gameLinkRaw");
        target.innerHTML = href;
      
        var params = (new URL(window.location)).searchParams;
        if (params.get("gameID") != gameID) {
          history.pushState({"gameID": gameID}, "Codenames", href);
        }
      }
      
      function renderFirstPlayerIndicator(firstPlayer) {
        var target = document.getElementById("firstPlayerIndicator");
        if (firstPlayer == "red") {
          target.innerHTML = 'The first player is red! <span class="dot red-dot"></span>';
        } else {
          target.innerHTML = 'The first player is blue! <span class="dot blue-dot"></span>';
        }
      }
      
      function renderAll(seed) {
        var prng = 1;
        var firstPlayer = determineFirstPlayer(prng);
        renderBoard(prng, firstPlayer);
        renderGameLink(seed);
        renderFirstPlayerIndicator(firstPlayer);
        updateRemainingIndicators();
      }
      
      /*
      document.addEventListener('DOMContentLoaded', function() {
        renderAll(getSeed());
      
        // setup Spymaster Button
        document.getElementById("spymasterButton").addEventListener("click", function(evt) {
          evt.stopPropagation();
          document.querySelectorAll(".gameword").forEach(function(elem) {
            elem.classList.add("spymaster");
          });
        });
        // setup New Game Button
        document.getElementById("newGameButton").addEventListener("click", function(evt) {
          evt.stopPropagation();
          renderAll(newSeed());
        });
        // setup Set Game ID Button
        document.getElementById("setGameIDButton").addEventListener("click", function(evt) {
          evt.stopPropagation();
          var answer = Number(window.prompt("Enter a gameID:"));
          if (answer != 0 && answer != null && !isNaN(answer)) {
            renderAll(answer);
          }
        });
        // setup popstate handler
        window.onpopstate = function(evt) {
          renderAll(evt.state.gameID);
        };
      });
      */




      
      // validation check on the data first for custom validation handling
      // then submit the form
      display_element.querySelector('#jspsych-codenames-next').addEventListener('click', function(){
        for(var i=0; i<trial.cards; i++){
          //if wrong team, show error message
          if(trial.questions[i].required){
            if(display_element.querySelector('#jspsych-codenames-'+i+' input:checked') == null){
              display_element.querySelector('#jspsych-codenames-'+i+' input').setCustomValidity(trial.required_message);
            } else {
              display_element.querySelector('#jspsych-codenames-'+i+' input').setCustomValidity('');
            }
          }
        }
        trial_form.reportValidity();
      })
  
      trial_form.addEventListener('submit', function(event) {
        event.preventDefault();
        // measure response time
        var endTime = performance.now();
        var response_time = endTime - startTime;
  
        // create object to hold responses
        var card_data = {};
        var has_response = [];
        for(var index=0; index<trial.cards; index++){
          var match = display_element.querySelector('#jspsych-codenames-'+index);
          var val = [];
          var inputboxes = match.querySelectorAll("input[type=checkbox]:checked")
          for(var j=0; j<inputboxes.length; j++){
            currentChecked = inputboxes[j];
            val.push(currentChecked.value)
          }
          var id = 'Q' + index
          var obje = {};
          var name = id;
          if(match.attributes['data-name'].value !== ''){
            name = match.attributes['data-name'].value;
          }
          obje[name] = val;
          Object.assign(card_data, obje);
          if(val.length == 0){ has_response.push(false); } else { has_response.push(true); }
        }
  
        // save data
        var trial_data = {
          rt: response_time,
          response: card_data,
          card_order: card_order
        };
        display_element.innerHTML = '';
  
        // next trial
        jsPsych.finishTrial(trial_data);
        
      });
  
      var startTime = performance.now();
    };
  
    return plugin;
  })();